#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <complex>
#include <pybind11/stl.h>

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

namespace py = pybind11;

PYBIND11_MODULE(rvrp_model, m) {
    py::class_<MadrichEngine>(m, "MadrichEngine")
            .def(py::init<>())
            .def("build_tour", &MadrichEngine::build_tour)
            .def("improve", &MadrichEngine::improve)
            .def("get_state", &MadrichEngine::get_state)
            .def("add_job", &MadrichEngine::add_job)
            .def("add_jobs", &MadrichEngine::add_jobs)
            .def("remove_job", &MadrichEngine::remove_job)
            .def("remove_jobs", &MadrichEngine::remove_jobs)
            .def("unassigned_jobs", &MadrichEngine::unassigned_jobs)
            .def("assigned_jobs", &MadrichEngine::assigned_jobs)
            .def("print", &MadrichEngine::print)
            .def_readwrite("storages", &MadrichEngine::storages)
            .def_readwrite("routes", &MadrichEngine::routes);
};
