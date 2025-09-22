cmake_minimum_required(VERSION 3.5)
project(pso_nmf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)   # if not found, use submodule or pip-installed pybind11's cmake
find_package(Eigen3 REQUIRED)

pybind11_add_module(pso_nmf pso_nmf.cpp)
target_include_directories(pso_nmf PRIVATE ${EIGEN3_INCLUDE_DIR})

