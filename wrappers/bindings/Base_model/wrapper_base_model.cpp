#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <complex>
#include <pybind11/stl.h>

#include "base_model.cpp"

namespace py = pybind11;

PYBIND11_MODULE(base_model, m) {
    py::class_<Window>(m, "Window")
            .def(py::init<>())
            .def(py::init<const Window &>())
            .def(py::init<std::tuple<time_t, time_t>>())
            .def(py::init<const std::string &, const std::string &>())
            .def("print", &Window::print)
            .def_readwrite("window", &Window::window);

    py::class_<Point>(m, "Point")
            .def(py::init<>())
            .def(py::init<const Point &>())
            .def(py::init<int, std::tuple<float, float>>())
            .def("print", &Point::print)
            .def_readwrite("matrix_id", &Point::matrix_id)
            .def_readwrite("point", &Point::point);

    py::class_<Cost>(m, "Cost")
            .def(py::init<>())
            .def(py::init<const Cost &>())
            .def(py::init<float, float, float>())
            .def("print", &Cost::print)
            .def_readwrite("start", &Cost::start)
            .def_readwrite("second", &Cost::second)
            .def_readwrite("meter", &Cost::meter);

    py::class_<Matrix>(m, "Matrix")
            .def(py::init<>())
            .def(py::init<const Matrix &>())
            .def(py::init<std::string, std::vector<std::vector<int>>, std::vector<std::vector<time_t>>>())
            .def(py::init<std::string, std::vector<std::vector<std::vector<int>>>, std::vector<std::vector<std::vector<time_t>>>, uint32_t, time_t, time_t>())
            .def("print", &Matrix::print)
            .def_readwrite("profile", &Matrix::profile)
            .def("get_distance", &Matrix::get_distance)
            .def("get_time", &Matrix::get_time);

    py::class_<State>(m, "State")
            .def(py::init<>())
            .def(py::init<const State &>())
            .def(py::init<time_t, int, float, std::optional<std::vector<int>>>())
            .def("__add__", &State::operator+, py::is_operator())
            .def("__iadd__", &State::operator+=, py::is_operator())
            .def("__sub__", &State::operator-, py::is_operator())
            .def("__isub__", &State::operator-=, py::is_operator())
            .def("__less__", &State::operator<, py::is_operator())
            .def("print", &State::print)
            .def_readwrite("travel_time", &State::travel_time)
            .def_readwrite("distance", &State::distance)
            .def_readwrite("cost", &State::cost)
            .def_readwrite("value", &State::value);

    py::class_<Job>(m, "Job")
            .def(py::init<>())
            .def(py::init<const Job &>())
            .def(py::init<int, std::string, std::vector<int>, std::vector<std::string>, const Point &, std::vector<Window>>())
            .def("__eq__", &Job::operator==, py::is_operator())
            .def("print", &Job::print)
            .def_readwrite("delay", &Job::delay)
            .def_readwrite("job_id", &Job::job_id)
            .def_readwrite("value", &Job::value)
            .def_readwrite("skills", &Job::skills)
            .def_readwrite("location", &Job::location)
            .def_readwrite("time_windows", &Job::time_windows);

    py::class_<Storage>(m, "Storage")
            .def(py::init<>())
            .def(py::init<int, std::string, std::vector<std::string>, const Point &, const Window &, Jobs>(),
                 py::arg("load") = 0,
                 py::arg("name"),
                 py::arg("skills"),
                 py::arg("location"),
                 py::arg("work_time"),
                 py::arg("unassigned_jobs")
            )
            .def(py::init<int, std::string, std::vector<std::string>, const Point &, const Window &>(),
                 py::arg("load") = 0,
                 py::arg("name"),
                 py::arg("skills"),
                 py::arg("location"),
                 py::arg("work_time")
            )
            .def("print", &Storage::print)
            .def_readwrite("load", &Storage::load)
            .def_readwrite("name", &Storage::name)
            .def_readwrite("skills", &Storage::skills)
            .def_readwrite("location", &Storage::location)
            .def_readwrite("work_time", &Storage::work_time)
            .def_readwrite("unassigned_jobs", &Storage::unassigned_jobs);

    py::class_<Courier>(m, "Courier")
            .def(py::init<>())
            .def(py::init<std::string, std::string, const Cost &, std::vector<int>, std::vector<std::string>, int, const Window &, const Point &, const Point>())
            .def(py::init<std::string, std::string, const Cost &, std::vector<int>, std::vector<std::string>, int, const Window &, const Point &, const Point &, Storages>())
            .def("__eq__", &Courier::operator==, py::is_operator())
            .def("print", &Courier::print)
            .def_readwrite("name", &Courier::name)
            .def_readwrite("profile", &Courier::profile)
            .def_readwrite("cost", &Courier::cost)
            .def_readwrite("value", &Courier::value)
            .def_readwrite("skills", &Courier::skills)
            .def_readwrite("max_distance", &Courier::max_distance)
            .def_readwrite("work_time", &Courier::work_time)
            .def_readwrite("start_location", &Courier::start_location)
            .def_readwrite("end_location", &Courier::end_location)
            .def_readwrite("storages", &Courier::storages);

    py::class_<Track>(m, "Track")
            .def(py::init<>())
            .def(py::init<ptrStorage>())
            .def("print", &Track::print)
            .def_readwrite("storage", &Track::storage)
            .def_readwrite("jobs", &Track::jobs);

    py::class_<Route>(m, "Route")
            .def(py::init<>())
            .def(py::init<uint16_t, time_t, bool, ptrCourier, const Matrix &>())
            .def("assigned_jobs", &Route::assigned_jobs)
            .def("unassigned_jobs", &Route::unassigned_jobs)
            .def("print", &Route::print)
            .def_readwrite("vec", &Route::vec)
            .def_readwrite("courier", &Route::courier)
            .def_readwrite("matrix", &Route::matrix)
            .def_readwrite("start_time", &Route::start_time)
            .def_readwrite("state", &Route::state)
            .def_readwrite("circle_track", &Route::circle_track)
            .def_readwrite("tracks", &Route::tracks);
};
