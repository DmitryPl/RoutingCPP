#include "engine.h"

#include <utility>
#include <algorithm>


MadrichEngine::MadrichEngine(Storages storages, uint32_t size, bool ignore_priority)
        : storages(std::move(storages)), routes(std::vector<Route>(size)), ignore_priority(ignore_priority) {}

MadrichEngine::MadrichEngine(
        uint16_t vec,
        Storages &storages,
        Couriers &couriers,
        std::map<std::string, Matrix> &matrices,
        bool circle_track,
        bool ignore_priority
)
        : storages(storages), routes(std::vector<Route>(couriers.size())), ignore_priority(ignore_priority) {
    std::size_t i = 0;
    for (const auto &courier : couriers) {
        Matrix &matrix = matrices[courier->profile];
        routes[i] = Route(vec, std::get<0>(courier->work_time.window), circle_track, courier, matrix);
        ++i;
    }
}

State MadrichEngine::get_state() const {
    State state;
    for (const auto &route : routes) {
        state += route.state;
    }
    return state;
}

std::size_t MadrichEngine::unassigned_jobs() const {
    std::size_t i = 0;
    for (const auto &storage: storages) {
        i += storage->unassigned_jobs.size();
    }
    return i;
}

std::size_t MadrichEngine::assigned_jobs() const {
    std::size_t i = 0;
    for (const auto &route : routes) {
        i += route.assigned_jobs();
    }
    return i;
}

void MadrichEngine::print() const {
    printf("MadrichEngine; storages: %zu, routes: %zu", storages.size(), routes.size());
}

[[maybe_unused]] void MadrichEngine::draw() const {
    printf("\nTour: %zu/%zu\n", assigned_jobs(), unassigned_jobs());
    for (const auto &route : routes) {
        printf("%zu\n", route.assigned_jobs());
        route.draw();
    }
}

[[maybe_unused]] void MadrichEngine::add_job(ptrJob &job, ptrStorage &storage) {
    set_zeros();
    auto it_storage = std::find(storages.begin(), storages.end(), storage);
    if (it_storage == storages.end()) {
        return;
    } else {
        auto index = std::distance(storages.begin(), it_storage);
        storages[index]->unassigned_jobs.emplace_back(job);
    }
}

[[maybe_unused]] void MadrichEngine::add_jobs(Jobs &jobs, ptrStorage &storage) {
    set_zeros();
    auto it_storage = std::find(storages.begin(), storages.end(), storage);
    if (it_storage == storages.end()) {
        return;
    } else {
        for (auto &job: jobs) {
            add_job(job, storage);
        }
    }
}

[[maybe_unused]] void MadrichEngine::remove_job(ptrJob &job, ptrStorage &storage) {
    set_zeros();
    auto it_storage = std::find(storages.begin(), storages.end(), storage);
    if (it_storage == storages.end()) {
        return;
    }
    auto here1 = std::find(storage->unassigned_jobs.begin(), storage->unassigned_jobs.end(), job);
    if (here1 != storage->unassigned_jobs.end()) {
        storage->unassigned_jobs.erase(storage->unassigned_jobs.begin(), here1);
        return;
    }
    for (auto &route : routes) {
        for (auto &track : route.tracks) {
            auto here2 = std::find(track.jobs.begin(), track.jobs.end(), job);
            if (here2 != track.jobs.end()) {
                track.jobs.erase(track.jobs.begin(), here2);
                return;
            }
        }
    }
}

[[maybe_unused]] void MadrichEngine::remove_jobs(Jobs &jobs, ptrStorage &storage) {
    set_zeros();
    for (auto &job : jobs) {
        remove_job(job, storage);
    }
}

std::tuple<uint32_t, uint32_t, time_t, int, float> MadrichEngine::generate_hash() const {
    State state = get_state();
    return {assigned_jobs(), unassigned_jobs(), state.travel_time, state.distance, state.cost};
}

void MadrichEngine::save_tour() {
    taboo_set.insert(generate_hash());
}

bool MadrichEngine::check_tour(const State &delta) {
    State state = get_state() - delta;
    std::tuple<uint32_t, uint32_t, time_t, int, float> hash(
            assigned_jobs(),
            unassigned_jobs(),
            state.travel_time,
            state.distance,
            state.cost
    );
    printf("Accepted: %d\n", !taboo_set.contains(hash));
    return !taboo_set.contains(hash);
}

bool MadrichEngine::get_from_copy(Route &route, Route &route_copy) {
    if (check_tour(route.state - route_copy.state)) {
        route.tracks = route_copy.tracks;
        route.state = route_copy.state;
        return true;
    }
    return false;
}

bool MadrichEngine::get_from_copy(Route &route1, Route &route1_copy, Route &route2, Route &route2_copy) {
    if (check_tour(route1.state - route1_copy.state) || check_tour(route2.state - route2_copy.state)) {
        route1.tracks = route1_copy.tracks;
        route1.state = route1_copy.state;
        route2.tracks = route2_copy.tracks;
        route2.state = route2_copy.state;
        return true;
    }
    return false;
}
