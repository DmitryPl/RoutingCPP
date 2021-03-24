cmake_minimum_required(VERSION 2.8.12)
project(hvrp_main)
set(PYBIND11_CPP_STANDARD /std:c++latest)

add_subdirectory(pybind11)
pybind11_add_module(hvrp_main wrapper_hvrp_main.cpp)

include_directories(../madrich)
include_directories(../madrich/examples)
include_directories(../madrich/local_search)
include_directories(../madrich/local_search/operators)
