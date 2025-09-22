// pso_nmf.cpp
// C++11 PSO to optimize H in ||X - W H||_F^2 with non-negativity projection
// Exposes PSO class to Python via pybind11.
// developed by Dr. Anirban Dey : email - anirban8895@gmail.com 


#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <Eigen/Dense>
#include <random>
#include <vector>
#include <limits>
#include <chrono>

namespace py = pybind11;
using Matrix = Eigen::MatrixXd;

struct PSOConfig {
    int population = 30;
    int max_iters = 500;
    double inertia = 0.729;   // common default
    double c1 = 1.49445;
    double c2 = 1.49445;
    double lb = 0.0; // H >= 0
    double ub = 10.0; // initial clamping upper bound for particles
    bool verbose = false;
    unsigned seed = 0;
};


class PSO {
public:
    PSO() { config_.seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()); }
    PSO(const PSOConfig &cfg) : config_(cfg) {
        if(config_.seed == 0) config_.seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    // fit: W (g x k), X (g x s) -> returns H (k x s)
    py::array_t<double> fit(py::array_t<double, py::array::c_style | py::array::forcecast> W_np,
                             py::array_t<double, py::array::c_style | py::array::forcecast> X_np) 
    {
        // Convert numpy -> Eigen
        auto bufW = W_np.request();
        auto bufX = X_np.request();
        if (bufW.ndim != 2 || bufX.ndim != 2) throw std::runtime_error("W and X must be 2-D arrays");

        int gW = (int)bufW.shape[0];
        int k = (int)bufW.shape[1];
        int gX = (int)bufX.shape[0];
        int s = (int)bufX.shape[1];

        if (gW != gX) throw std::runtime_error("W and X must have same number of rows (features)");

        // Map data
        const double *ptrW = static_cast<double *>(bufW.ptr);
        const double *ptrX = static_cast<double *>(bufX.ptr);

        Matrix W = Eigen::Map<const Matrix>(ptrW, gW, k);
        Matrix X = Eigen::Map<const Matrix>(ptrX, gX, s);

        // dimension of particle = k * s
        int dim = k * s;
        int pop = config_.population;
        if (pop < 2) pop = 2;

        // RNG
        std::mt19937 rng(config_.seed);
        std::uniform_real_distribution<double> unif01(0.0, 1.0);
        std::uniform_real_distribution<double> initDist(config_.lb, config_.ub);

        // particle structures
        std::vector<std::vector<double>> pos(pop, std::vector<double>(dim));
        std::vector<std::vector<double>> vel(pop, std::vector<double>(dim));
        std::vector<std::vector<double>> pbest_pos(pop, std::vector<double>(dim));
        std::vector<double> pbest_cost(pop, std::vector<double>::size_type(0)); // will set later
        std::vector<double> cost(pop);

        // initialize
        double global_best_cost = std::numeric_limits<double>::infinity();
        std::vector<double> gbest_pos(dim);

        // helpful lambda: evaluate cost given flattened H
        auto eval_cost = [&](const std::vector<double> &flatH)->double {
            // map to Eigen
            Matrix H = Matrix::Zero(k, s);
            for (int j = 0; j < s; ++j) {
                for (int i = 0; i < k; ++i) {
                    double v = flatH[j*k + i];
                    if (v < config_.lb) v = config_.lb;
                    H(i,j) = v;
                }
            }
            Matrix R = X - W * H;
            double f = R.squaredNorm(); // Frobenius norm squared
            return f;
        };

        // initialize particles
        for (int i = 0; i < pop; ++i) {
            for (int d = 0; d < dim; ++d) {
                pos[i][d] = initDist(rng);
                vel[i][d] = (initDist(rng) - initDist(rng)) * 0.1; // small initial vel
                pbest_pos[i][d] = pos[i][d];
            }
            pbest_cost[i] = eval_cost(pos[i]);
            cost[i] = pbest_cost[i];
            if (pbest_cost[i] < global_best_cost) {
                global_best_cost = pbest_cost[i];
                gbest_pos = pbest_pos[i];
            }
        }

        // PSO loop
        for (int iter = 0; iter < config_.max_iters; ++iter) {
            for (int i = 0; i < pop; ++i) {
                // update velocity and position
                for (int d = 0; d < dim; ++d) {
                    double r1 = unif01(rng);
                    double r2 = unif01(rng);
                    vel[i][d] = config_.inertia * vel[i][d]
                                + config_.c1 * r1 * (pbest_pos[i][d] - pos[i][d])
                                + config_.c2 * r2 * (gbest_pos[d] - pos[i][d]);
                    pos[i][d] += vel[i][d];
                    // projection for non-negativity and bounds
                    if (pos[i][d] < config_.lb) pos[i][d] = config_.lb;
                    // no strict upper bound, but clamp to ub to keep numeric stable
                    if (pos[i][d] > config_.ub) pos[i][d] = config_.ub;
                }

                // evaluate
                double f = eval_cost(pos[i]);
                cost[i] = f;
                if (f < pbest_cost[i]) {
                    pbest_cost[i] = f;
                    pbest_pos[i] = pos[i];
                    if (f < global_best_cost) {
                        global_best_cost = f;
                        gbest_pos = pos[i];
                    }
                }
            }

            if (config_.verbose && (iter % 50 == 0 || iter == config_.max_iters - 1)) {
                py::print("[PSO] iter:", iter, "best_cost:", global_best_cost);
            }
        }

        // Build H from gbest_pos (k x s)
        // Note flattened format used: index = j*k + i  (column-major by columns j)
        py::array_t<double> H_out({k, s});
        auto bufH = H_out.request();
        double *ptrH = static_cast<double *>(bufH.ptr);
        for (int j = 0; j < s; ++j) {
            for (int i = 0; i < k; ++i) {
                ptrH[j*k + i] = gbest_pos[j*k + i]; // column-major mapping
            }
        }

        return H_out;
    }

    // expose config setters/getters
    void set_population(int p) { config_.population = p; }
    void set_max_iters(int m) { config_.max_iters = m; }
    void set_inertia(double w) { config_.inertia = w; }
    void set_c1(double c1) { config_.c1 = c1; }
    void set_c2(double c2) { config_.c2 = c2; }
    void set_bounds(double lb, double ub) { config_.lb = lb; config_.ub = ub; }
    void set_verbose(bool v) { config_.verbose = v; }
    void set_seed(unsigned s) { config_.seed = s; }

private:
    PSOConfig config_;
};

PYBIND11_MODULE(pso_nmf, m) {
    m.doc() = "PSO-based H initializer for NMF (C++11 + pybind11)";

    py::class_<PSO>(m, "PSO")
        .def(py::init<>(), "Create default PSO object")
        .def("fit", &PSO::fit, R"pbdoc(
            Fit PSO to find H given W and X.
            W: (g x k) numpy array
            X: (g x s) numpy array
            returns: H (k x s) numpy array
        )pbdoc")
        .def("set_population", &PSO::set_population)
        .def("set_max_iters", &PSO::set_max_iters)
        .def("set_inertia", &PSO::set_inertia)
        .def("set_c1", &PSO::set_c1)
        .def("set_c2", &PSO::set_c2)
        .def("set_bounds", &PSO::set_bounds)
        .def("set_verbose", &PSO::set_verbose)
        .def("set_seed", &PSO::set_seed)
        ;
}

