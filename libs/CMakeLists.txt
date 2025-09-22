cmake_minimum_required(VERSION 3.12)
project(pso_nmf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pybind11
find_package(pybind11 REQUIRED)

# Find Eigen
find_package(Eigen3 3.3 REQUIRED)

# Build the module
pybind11_add_module(pso_nmf pso_nmf.cpp)

# Link Eigen
target_link_libraries(pso_nmf PRIVATE Eigen3::Eigen)
