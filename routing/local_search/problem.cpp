#include "problem.h"
#include <ranges>


MadrichEngine RvrpProblem::init_tour(
        uint16_t vec,
        Storages &storages,
        Couriers &couriers,
        std::map<std::string, Matrix> &matrices,
        bool circle_track
) {
    MadrichEngine tour(storages, couriers.size());
    printf("\nCreating MadrichEngine, Couriers: %zu, Jobs: %lu\n", couriers.size(), tour.unassigned_jobs());

    uint32_t i = 0;
    for (auto &courier : couriers) {
        tour.routes[i++] = init_route(vec, courier, matrices, circle_track);
    }

    printf("Created MadrichEngine, Routes: %zu, Assigned: %lu\n\n", tour.routes.size(), tour.assigned_jobs());
    return tour;
}

Route RvrpProblem::init_route(
        uint16_t vec,
        ptrCourier &courier,
        std::map<std::string, Matrix> &matrices,
        bool circle_track
) {
    printf("Creating Route, Courier: %s, type: %s\n", courier->name.c_str(), courier->profile.c_str());
    Matrix &matrix = matrices[courier->profile];
    Route route(vec, std::get<0>(courier->work_time.window), circle_track, courier, matrix);
    printf("Unassigned: %lu\n", route.unassigned_jobs());
    int curr_point = courier->start_location.matrix_id;
    State state(0, 0, courier->cost.start);

    while (true) {
        state.value = std::vector<int>(vec);  // создаем новый подмаршрут
        auto tmp = init_track(curr_point, state, route);  // с одной точкой
        if (!tmp) { break; }
        auto&[new_state, track] = tmp.value();
        state = new_state;
        curr_point = track.jobs[0]->location.matrix_id;
        printf(".");

        while (true) {  // ищем новые точки
            std::optional answer = choose_job(curr_point, state, track, route);
            if (!answer) { break; }
            state = answer.value();
            curr_point = track.jobs.back()->location.matrix_id;
            printf(".");
        }  // больше нет доступных точек

        if (circle_track) {  // возможно обязаны вернуться на склад
            auto answer = go_storage(curr_point, state, track.storage, route);
            state += answer.value();  // учитывается в выборе точки
            curr_point = track.storage->location.matrix_id;
        }

        printf("\n");
        route.tracks.push_back(track);
    }

    if (route.tracks.empty()) {
        return route;
    }
    auto answer = end(curr_point, state, route);
    state += answer.value();
    route.state += state;
    printf("\nCreated Route, Jobs: %lu/%lu\n", route.assigned_jobs(), route.unassigned_jobs());
    return route;
}

std::optional<std::tuple<State, Track>>
RvrpProblem::init_track(int current_point, const State &state, Route &route) {
    std::vector states = sorted_storages(current_point, state, route);
    if (states.empty()) {
        return std::nullopt;
    }

    for (const auto &st : states) {
        int storage_id = std::get<1>(st);
        const ptrStorage &storage = route.courier->storages[storage_id];  // едем на склад
        std::optional answer = go_storage(current_point, state, storage, route);
        if (!answer) {
            continue;
        }
        Track track(storage);  // пытаемся найти хоть одну доступную задачу
        answer = choose_job(storage->location.matrix_id, state + answer.value(), track, route);
        if (answer) {
            return std::tuple(answer.value(), track);
        }
    }

    return std::nullopt;
}

std::optional<State> RvrpProblem::choose_job(int location, const State &state, Track &track, Route &route) {
    std::optional<State> best_state = std::nullopt;
    const ptrStorage &storage = track.storage;
    std::size_t size = storage->unassigned_jobs.size();
    int index = -1;

    for (std::size_t i = 0; i < size; ++i) {
        const ptrJob &job = storage->unassigned_jobs[i];  // едем на задачу
        std::optional answer = go_job(location, state, job, storage, route);
        if (!answer) { continue; }
        State new_state = state + answer.value();
        State end_track = new_state;
        int current_point = job->location.matrix_id;

        if (!(!best_state || new_state < best_state)) { continue; }

        if (route.circle_track) {  // тогда нам нужно вернуться на склад
            answer = go_storage(job->location.matrix_id, new_state, storage, route);
            if (!answer) { continue; }
            end_track += answer.value();
            current_point = track.storage->location.matrix_id;
        }

        if (end(current_point, end_track, route)) {  // и всегда должна быть возможность закончить
            best_state = new_state;
            index = i;
        }
    }

    if (index == -1) {
        return std::nullopt;
    }
    track.jobs.push_back(storage->unassigned_jobs[index]);
    storage->unassigned_jobs.erase(storage->unassigned_jobs.begin() + index);
    return best_state;
}

std::optional<State> RvrpProblem::get_state(const Route &route) {
    auto size_t = route.tracks.size();
    if (size_t == 0) {
        State st(0, 0, 0.0);
        return st;
    }

    int curr_point = route.courier->start_location.matrix_id;  // старт
    State state;

    for (const auto &track : route.tracks) {
        if (track.jobs.empty()) {
            continue;
        }

        state.value = std::vector<int>(route.vec);
        auto answer = go_storage(curr_point, state, track.storage, route);
        if (!answer) {
            return std::nullopt;
        }
        state += answer.value();
        curr_point = track.storage->location.matrix_id;  // едем на склад

        for (const auto &job : track.jobs) {
            answer = go_job(curr_point, state, job, track.storage, route);
            if (!answer) {
                return std::nullopt;
            }
            state += answer.value();
            curr_point = job->location.matrix_id;  // едем на каждую задачу
        }

        if (route.circle_track) {
            answer = go_storage(curr_point, state, track.storage, route);
            if (!answer) {
                return std::nullopt;
            }
            curr_point = track.storage->location.matrix_id;  // возможно возвращаемся
            state += answer.value();
        }
    }

    if (!validate_courier(state, route)) {
        return std::nullopt;
    }
    auto answer = end(curr_point, state, route);  // конечная точка
    if (!answer) {
        return std::nullopt;
    }
    state += answer.value();
    state.value = std::nullopt;
    return state;
}

std::vector<std::tuple<time_t, std::size_t>>
RvrpProblem::sorted_storages(int curr_point, const State &state, const Route &route) {
    const Matrix &matrix = route.matrix;
    std::vector<std::tuple<time_t, std::size_t>> states;

    for (std::size_t i = 0; i < route.courier->storages.size(); ++i) {
        if (route.courier->storages[i]->unassigned_jobs.empty()) {  // отсекаем пустые
            continue;
        }
        time_t tt = matrix.get_time(curr_point, route.courier->storages[i]->location.matrix_id);
        states.emplace_back(tt, i);
    }

    sort(states.begin(), states.end());
    return states;
}

bool RvrpProblem::validate_skills(const ptrJob &job, const ptrCourier &courier) {
    return std::ranges::all_of(
            job->skills.begin(),
            job->skills.end(),
            [&courier](const std::string &skill) {
                return std::ranges::find(courier->skills, skill) != courier->skills.end();
            }
    );
}

bool RvrpProblem::validate_skills(const ptrStorage &storage, const ptrCourier &courier) {
    return std::ranges::all_of(
            storage->skills.begin(),
            storage->skills.end(),
            [&courier](const std::string &skill) {
                return std::ranges::find(courier->skills, skill) != courier->skills.end();
            }
    );
}

bool RvrpProblem::validate_courier(const State &state, const Route &route) {
    // 1. проверяем время работы
    const auto&[start_shift, end_shift] = route.courier->work_time.window;
    time_t start_time = route.start_time;
    if (!((start_shift <= start_time + state.travel_time) && (start_time + state.travel_time <= end_shift))) {
        return false;
    }
    // 2. проверяем максимальную дистанцию
    int max_distance = route.courier->max_distance;
    if (max_distance != 0 && state.distance > max_distance) {
        return false;
    }
    // 3. проверяем на перевес
    if (!state.value) {
        return true;
    }
    auto size_v = state.value.value().size();
    for (std::size_t i = 0; i < size_v; ++i) {
        if (state.value.value()[i] > route.courier->value[i]) {
            return false;
        }
    }
    return true;
}

bool RvrpProblem::validate_storage(const ptrStorage &storage, const ptrCourier &courier) {
    return std::ranges::find(courier->storages, storage) != courier->storages.end();
}

std::optional<State>
RvrpProblem::go_job(
        int curr_point,
        const State &state,
        const ptrJob &job,
        const ptrStorage &storage,
        const Route &route
) {
    if (!RvrpProblem::validate_skills(job, route.courier)) {
        return std::nullopt;
    }
    // доехать + отдать заказ
    time_t tt = route.matrix.get_time(curr_point, job->location.matrix_id) + job->delay;
    int d = route.matrix.get_distance(storage->location.matrix_id, job->location.matrix_id);
    // возможно придется подождать
    time_t waiting = RvrpProblem::waiting(state.travel_time + tt, route.start_time, job->time_windows);
    if (waiting == -1) {
        return std::nullopt;
    }
    tt += waiting;
    State tmp(tt, d, RvrpProblem::cost(tt, d, route), std::optional<std::vector<int>>(job->value));
    // курьер уложится по времени, расстоянию, грузу?
    if (!RvrpProblem::validate_courier(tmp + state, route)) {
        return std::nullopt;
    }
    return tmp;
}

std::optional<State>
RvrpProblem::go_storage(int curr_point, const State &state, const ptrStorage &storage, const Route &route) {
    if (!RvrpProblem::validate_skills(storage, route.courier)) {
        return std::nullopt;
    }
    // доехать + перезагрузиться
    time_t tt = route.matrix.get_time(curr_point, storage->location.matrix_id) + storage->load;
    int d = route.matrix.get_distance(storage->location.matrix_id, storage->location.matrix_id);
    // подождать до открытия
    time_t waiting = RvrpProblem::waiting(state.travel_time + tt, route.start_time, storage->work_time);
    if (waiting == -1) {
        return std::nullopt;
    }
    tt += waiting;
    State tmp(tt, d, RvrpProblem::cost(tt, d, route));
    return tmp;
}

float RvrpProblem::cost(time_t travel_time, int distance, const Route &route) {
    return float(travel_time) * route.courier->cost.second + float(distance) * route.courier->cost.meter;
}

time_t RvrpProblem::waiting(time_t arrival_time, time_t start_time, const std::vector<Window> &time_windows) {
    time_t waiting = -1;

    for (const auto &window : time_windows) {
        const auto&[start_shift, end_shift] = window.window;
        if (start_shift <= start_time + arrival_time <= end_shift) {
            return 0;  // точное попадание в окно
        }
        time_t new_waiting = start_shift - (start_time + arrival_time);
        if (new_waiting > 0) {
            if (waiting == -1 || new_waiting < waiting) {
                waiting = new_waiting;  // минимальное положительное
            }
        }
    }

    if (waiting == -1) {
        return -1;
    }
    return waiting;
}

time_t RvrpProblem::waiting(time_t arrival_time, time_t start_time, const Window &time_window) {
    const auto&[start_shift, end_shift] = time_window.window;
    if (start_shift <= start_time + arrival_time <= end_shift) {
        return 0;  // точное попадание
    }
    time_t waiting = start_shift - (start_time + arrival_time);
    return waiting > 0 ? static_cast<int>(waiting) : -1;  // есть ли смысл вообще ждать
}

State RvrpProblem::get_state_track(const Track &track, const Route &route) {
    int location = track.storage->location.matrix_id;
    State state;
    for (const auto &job : track.jobs) {
        time_t tt = route.matrix.get_time(location, job->location.matrix_id) + job->delay;
        int d = route.matrix.get_distance(location, job->location.matrix_id);
        float c = cost(tt, d, route);
        location = job->location.matrix_id;
        state += State(tt, d, c, job->value);
    }
    if (route.circle_track) {  // не забываем про такую возможность
        state.travel_time += route.matrix.get_time(location, track.storage->location.matrix_id);
        state.distance += route.matrix.get_distance(location, track.storage->location.matrix_id);
    }
    return state;
}

std::optional<State> RvrpProblem::end(int curr_point, const State &state, const Route &route) {
    int end_id = route.courier->end_location.matrix_id;
    time_t tt = route.matrix.get_time(curr_point, end_id);
    int d = route.matrix.get_distance(curr_point, end_id);
    State st(tt, d, cost(tt, d, route));
    if (!validate_courier(state + st, route)) {
        return std::nullopt;
    }
    return st;
}
