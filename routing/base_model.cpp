#include "base_model.h"

#include <utility>


//// Window


Window::Window(std::tuple<time_t, time_t> window) : window(std::move(window)) {}

time_t to_seconds(const std::string &time) {
    std::tm t{};
    std::istringstream ss(time);
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    return std::mktime(&t);
}

Window::Window(const std::string &start_t, const std::string &end_t) {
    window = std::tuple<time_t, time_t>(to_seconds(start_t), to_seconds(end_t));
}

[[maybe_unused]] void Window::print() const {
    printf("start: %ld, end: %ld\n", std::get<0>(window), std::get<1>(window));
}


//// Point


Point::Point(int matrix_id, std::tuple<float, float> point)
        : matrix_id(matrix_id), point(std::move(point)) {}

[[maybe_unused]] void Point::print() const {
    printf("Point: %f %f id: %d\n", std::get<0>(point), std::get<1>(point), matrix_id);
}


//// Cost


Cost::Cost(float start, float second, float meter) : start(start), second(second), meter(meter) {}

[[maybe_unused]] void Cost::print() const {
    printf("%f %f %f\n", second, meter, start);
}


//// Matrix


Matrix::Matrix(
        std::string profile,
        std::vector<std::vector<int>> distance,
        std::vector<std::vector<time_t>> travel_time
)
        : profile(std::move(profile)), distance({std::move(distance)}), travel_time({std::move(travel_time)}) {}

Matrix::Matrix(
        std::string profile,
        std::vector<std::vector<std::vector<int>>> distance,
        std::vector<std::vector<std::vector<time_t>>> travel_time,
        uint32_t discreteness,
        time_t start_time,
        time_t end_time
)
        : profile(std::move(profile)), distance(std::move(distance)), travel_time(std::move(travel_time)),
          discreteness(discreteness), start_time(start_time), end_time(end_time) {}

[[maybe_unused]] void Matrix::print() const {
    printf("Matrix: %s, size: %zu\n", profile.c_str(), distance.size());
}

time_t Matrix::get_time(uint32_t src, uint32_t dst, time_t curr_time) const {
    if (end_time < curr_time) { return -1; }
    uint32_t matrix = start_time == 0 ? 0 : (curr_time - start_time) / discreteness;
    return travel_time[matrix][src][dst];
}

int Matrix::get_distance(uint32_t src, uint32_t dst, time_t curr_time) const {
    if (end_time < curr_time) { return -1; }
    uint32_t matrix = start_time == 0 ? 0 : (curr_time - start_time) / discreteness;
    return distance[matrix][src][dst];
}


//// State


State::State(time_t travel_time, int distance, float cost, std::optional<std::vector<int>> value)
        : travel_time(travel_time), distance(distance), cost(cost), value(std::move(value)) {}

[[maybe_unused]] void State::print() const {
    printf("State; travel time: %ld, distance: %d, cost: %f\n", travel_time, distance, cost);
}

std::optional<std::vector<int>> State::sum_values(const State &lt, const State &rt) {
    if ((!lt.value.has_value()) && (!rt.value.has_value())) {
        return std::nullopt;
    } else if (lt.value.has_value() && !rt.value.has_value()) {
        return lt.value;
    } else if (!lt.value.has_value() && rt.value.has_value()) {
        return rt.value;
    } else {
        int size = lt.value.value().size();
        std::optional ret = std::vector<int>(size);
        std::vector<int> ltv = lt.value.value();
        std::vector<int> rtv = rt.value.value();
        for (std::size_t i = 0; i < size; ++i) {
            ret.value()[i] = ltv[i] + rtv[i];
        }
        return ret;
    }
}

State State::operator+(const State &rhs) const {
    return State(travel_time + rhs.travel_time, distance + rhs.distance, cost + rhs.cost, sum_values(*this, rhs));
}

State &State::operator+=(const State &rhs) {
    travel_time += rhs.travel_time;
    distance += rhs.distance;
    cost += rhs.cost;
    value = sum_values(*this, rhs);
    return *this;
}

bool State::operator<(const State &rhs) const {
    if (travel_time != rhs.travel_time) {
        return travel_time < rhs.travel_time;
    }
    if (cost != rhs.cost) {
        return cost < rhs.cost;
    }
    if (distance != rhs.distance) {
        return distance < rhs.distance;
    }
    return false;
}

State &State::operator-=(const State &rhs) {
    travel_time -= rhs.travel_time;
    distance -= rhs.distance;
    cost -= rhs.cost;
    value = std::nullopt;
    return *this;
}

State State::operator-(const State &rhs) const {
    return State(travel_time - rhs.travel_time, distance - rhs.distance, cost - rhs.cost);
}


//// Job


Job::Job(
        int delay,
        std::string job_id,
        std::vector<int> value,
        std::vector<std::string> skills,
        const Point &location,
        std::vector<Window> time_windows
)
        : job_id(std::move(job_id)), delay(delay), value(std::move(value)),
          skills(std::move(skills)), location(location), time_windows(std::move(time_windows)) {}

Job::Job(
        int delay,
        int priority,
        std::string job_id,
        std::vector<int> value,
        std::vector<std::string> skills,
        const Point &location, std::vector<Window> time_windows
)
        : job_id(std::move(job_id)), delay(delay), value(std::move(value)), priority(priority),
          skills(std::move(skills)), location(location), time_windows(std::move(time_windows)) {}

[[maybe_unused]] void Job::print() const {
    printf("Job id: %s, priority: %d\n", job_id.c_str(), priority);
}

bool Job::operator==(const Job &other) const {
    return other.job_id == job_id;
}


//// Storage


Storage::Storage(
        int load,
        std::string name,
        std::vector<std::string> skills,
        const Point &location,
        const Window &work_time,
        Jobs unassigned_jobs
)
        : load(load), name(std::move(name)), skills(std::move(skills)),
          location(location), work_time(work_time), unassigned_jobs(std::move(unassigned_jobs)) {}

Storage::Storage(
        int load,
        std::string name,
        std::vector<std::string> skills,
        const Point &location,
        const Window &work_time
)
        : load(load), name(std::move(name)), skills(std::move(skills)),
          location(location), work_time(work_time) {}

[[maybe_unused]] void Storage::print() const {
    printf("Storage: %s\n", name.c_str());
}


//// Courier


Courier::Courier(
        std::string name,
        std::string profile,
        const Cost &cost,
        std::vector<int> value,
        std::vector<std::string> skills,
        int max_distance,
        const Window &work_time,
        const Point &start_location,
        const Point &end_location
)
        : name(std::move(name)), profile(std::move(profile)), cost(cost), value(std::move(value)),
          skills(std::move(skills)), max_distance(max_distance), work_time(work_time),
          start_location(start_location), end_location(end_location) {}

Courier::Courier(
        std::string name,
        std::string profile,
        const Cost &cost,
        std::vector<int> value,
        std::vector<std::string> skills,
        int max_distance,
        const Window &work_time,
        const Point &start_location,
        const Point &end_location,
        Storages storages
)
        : name(std::move(name)), profile(std::move(profile)), cost(cost), value(std::move(value)),
          skills(std::move(skills)), max_distance(max_distance), work_time(work_time),
          start_location(start_location), end_location(end_location), storages(std::move(storages)) {}

[[maybe_unused]] void Courier::print() const {
    printf("Courier; name: %s\n", name.c_str());
}

bool Courier::operator==(const Courier &other) const {
    return name == other.name;
}


//// Track


[[maybe_unused]] void Track::print() const {
    printf("Track; storage: %s, jobs: %llu", storage->name.c_str(), jobs.size());
}


//// Route


std::size_t Route::assigned_jobs() const {
    std::size_t i = 0;
    for (const auto &track : tracks) {
        i += track.jobs.size();
    }
    return i;
}

std::size_t Route::unassigned_jobs() const {
    std::size_t i = 0;
    for (const auto &storage : courier->storages) {
        i += storage->unassigned_jobs.size();
    }
    return i;
}

void Route::print() const {
    printf("Route; courier: %s, tracks: %llu, jobs: %llu",
           courier->name.c_str(), tracks.size(), assigned_jobs());
}

Route::Route(uint16_t vec, time_t start_time, bool circle_track, ptrCourier courier, const Matrix &matrix)
        : vec(vec), start_time(start_time), courier(std::move(courier)), matrix(matrix), circle_track(circle_track) {}

[[maybe_unused]] void Route::draw() const {
    for (const auto &track : tracks) {
        printf("%llu\t", track.jobs.size());
        for (const auto &job : track.jobs) {
            printf(".");
        }
        printf("\n");
    }
    printf("\n");
}
