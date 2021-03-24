cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

add_madrich_executable(HvrpRunner
  SOURCES hvrp_main.cpp
)

add_madrich_executable(VrpRunner
  SOURCES vrp_main.cpp
)

add_madrich_executable(RvrpRunner
  SOURCES rvrp_main.cpp
)

