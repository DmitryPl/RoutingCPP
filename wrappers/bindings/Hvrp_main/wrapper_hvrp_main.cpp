#include <pybind11/pybind11.h>

#include "base_model.cpp"
#include "insert_best.cpp"
#include "engine.cpp"
#include "ruin.cpp"
#include "block_route.cpp"
#include "improve_tour.cpp"
#include "route_utils.cpp"
#include "problem.cpp"
#include "intra_operators.cpp"
#include "inter_operators.cpp"
#include "hvrp_main.cpp"

namespace py = pybind11;

PYBIND11_MODULE(hvrp_main, m) {
    m.def("hvrp_main", &main);
};
