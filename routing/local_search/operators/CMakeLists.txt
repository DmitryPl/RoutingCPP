cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

add_madrich_library(local_search_operators
  SOURCES route_utils.cpp
          inter_operators.cpp
          intra_operators.cpp
  HEADERS route_utils.h
          inter_operators.h
          intra_operators.h
)

