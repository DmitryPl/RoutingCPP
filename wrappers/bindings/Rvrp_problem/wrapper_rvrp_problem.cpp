#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <complex>
#include <pybind11/stl.h>

#include "base_model.cpp"
#include "engine.cpp"
#include "problem.cpp"

namespace py = pybind11;

PYBIND11_MODULE(rvrp_problem, m) {
    py::class_<RvrpProblem>(m, "RvrpProblem")
            .def(py::init<>())
            .def("init_tour", &RvrpProblem::init_tour)
            .def("init_route", &RvrpProblem::init_route)
            .def("init_track", &RvrpProblem::init_track)
            .def("get_state", &RvrpProblem::get_state)
            .def("get_state_track", &RvrpProblem::get_state_track);
};
