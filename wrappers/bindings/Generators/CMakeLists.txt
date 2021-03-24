cmake_minimum_required(VERSION 2.8.12)
project(generators)
set(PYBIND11_CPP_STANDARD /std:c++latest)

add_subdirectory(pybind11)
pybind11_add_module(generators wrapper_generators.cpp)

include_directories(../madrich)
include_directories(../madrich/examples)
include_directories(../madrich/local_search)
include_directories(../madrich/local_search/operators)
