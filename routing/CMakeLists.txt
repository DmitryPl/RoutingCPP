cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/madrich.cmake)

add_madrich_library(base_model
  SOURCES base_model.cpp
  HEADERS base_model.h
)

add_madrich_library(generators
  SOURCES generators.cpp
  HEADERS generators.h
)

add_subdirectory(local_search)
add_subdirectory(examples)

