#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "base_model.cpp"
#include "generators.cpp"

namespace py = pybind11;

PYBIND11_MODULE(generators, m) {
    m.def("generate_value", &generate_value);
    m.def("generate_tuple", &generate_tuple);
    m.def("generate_points", &generate_points);
    m.def("generate_distance", &generate_distance);
    m.def("generate_time", &generate_time);
    m.def("generate_jobs", &generate_jobs);
    m.def("generate_rvrp", &generate_rvrp);
};
