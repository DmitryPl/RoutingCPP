cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

add_madrich_library(local_search
  SOURCES problem.cpp
          engine.cpp
          improve_tour.cpp
          block_route.cpp
          insert_best.cpp
          ruin.cpp
  HEADERS problem.h
          engine.h
)

add_subdirectory(operators)

