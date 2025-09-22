# PSO-NMF Library

A **C++11 Particle Swarm Optimization (PSO)** library for **Non-negative Matrix Factorization (NMF)**, exposed to Python via **pybind11**. Optimizes the `H` matrix in `\( X \approx W H \)` while enforcing non-negativity.

---

## Features

- Fast C++ implementation using **Eigen**  
- Python interface via **pybind11**  
- Non-negative constraint on `H`  
- Configurable PSO parameters: population, inertia, cognitive/social coefficients, bounds, iterations  
- Verbose mode for monitoring optimization  
- Supports SVD-based or custom `W` initialization  

---

## Installation

```bash
sudo apt-get install pybind11-dev libeigen3-dev cmake g++
cd PSO-BADINIT/libs/build
cmake ..
make
