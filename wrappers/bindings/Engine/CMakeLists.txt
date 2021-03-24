cmake_minimum_required(VERSION 2.8.12)
project(rvrp_model)
set(PYBIND11_CPP_STANDARD /std:c++latest)

add_subdirectory(pybind11)
pybind11_add_module(engine wrapper_engine.cpp)

include_directories(../madrich)
include_directories(../madrich/examples)
include_directories(../madrich/local_search)
include_directories(../madrich/local_search/operators)
