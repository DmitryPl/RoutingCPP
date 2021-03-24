#include "engine.h"

#include <local_search/operators/route_utils.h>
#include <local_search/problem.h>

using std::optional;
typedef std::tuple<State, int, int, int> answer_t;


void MadrichEngine::build_tour() {
    check_block();
    unassigned_insert();
}

uint32_t MadrichEngine::max_priority() const {
    uint32_t max = 0;
    for (const auto &storage : storages) {
        for (const auto &job : storage->unassigned_jobs) {
            max = job->priority > max ? job->priority : max;
        }
    }
    return max;
}

bool MadrichEngine::insert_best(uint32_t current_priority) {
    int storage_id, job_id;
    char operation;
    answer_t best_answer;
    bool changed = false;
    optional<State> best_state = std::nullopt;
    auto storages_size = storages.size();

    for (int i = 0; i < storages_size; ++i) {
        ptrStorage &storage = storages[i];
        auto jobs_size = storage->unassigned_jobs.size();

        for (int j = 0; j < jobs_size; ++j) {
            ptrJob &job = storage->unassigned_jobs[j];
            if (!ignore_priority and job->priority != current_priority) {
                continue;
            }

            std::optional answer = choose_best(best_state, job, storage);
            if (!answer) {
                continue;
            }
            const auto&[o_tmp, a_tmp] = answer.value();
            operation = o_tmp;
            best_state = std::get<0>(a_tmp);
            best_answer = a_tmp;
            changed = true;
            storage_id = i;
            job_id = j;
        }
    }

    if (changed) {
        const auto&[_, a, b, c] = best_answer;
        Route &route = routes[a];
        ptrStorage &storage = storages[storage_id];
        ptrJob &job = storage->unassigned_jobs[job_id];
        mark_route(true, route);

        if (operation == 's') {  // или тупо вставка трека в маршрут
            route.tracks.insert(route.tracks.begin() + b, Track(job, storage));
        } else if (operation == 'f') {  // или вставляем задачу в правильное место в существующем треке
            Jobs jobs = route.tracks[b].jobs;
            std::vector new_jobs = insert(c, job, jobs);
            route.tracks[b].jobs = new_jobs;
        } else {
            printf("incorrect operation\n");
            return false;
        }

        route.state = RvrpProblem::get_state(route).value();
        storage->unassigned_jobs.erase(storage->unassigned_jobs.begin() + job_id);
        printf("Inserted\n");
    }

    return changed;
}

bool MadrichEngine::unassigned_insert() {
    printf("\nUnassigned insert started\n");
    bool result = false;
    bool changed = true;
    uint32_t max = ignore_priority ? 0 : max_priority();
    uint32_t curr = 0;

    while (changed || (!ignore_priority && curr <= max)) {
        changed = insert_best(curr);
        if (!changed && (!ignore_priority && curr <= max)) {
            ++curr;
        }
    }

    printf("Ended\n");
    return result;
}

optional<std::tuple<char, answer_t>>
MadrichEngine::choose_best(const optional<State> &best_state, const ptrJob &job, const ptrStorage &storage) {
    char operation;
    answer_t value;

    std::optional first = insert_job(job, storage);  // вставка в трек
    std::optional second = insert_track(job, storage);  // вставка трека

    if (!first && !second) {
        return std::nullopt;
    } else if (!second) {
        operation = 'f';
        value = first.value();
    } else if (!first) {
        operation = 's';
        value = second.value();
    } else {  // выбираем по меньшему state
        if (std::get<0>(first.value()) < std::get<0>(second.value())) {
            operation = 'f';
            value = first.value();
        } else {
            operation = 's';
            value = second.value();
        }
    }

    if (!best_state || std::get<0>(value) < best_state) {
        return std::make_tuple(operation, value);
    }
    return std::nullopt;
}

bool check_value(const ptrJob &job, const Track &track, const Route &route) {
    State track_state = RvrpProblem::get_state_track(track, route);
    bool flag = true;
    for (uint32_t k = 0; k < route.vec; ++k) {  // проверка на переполнение
        if (track_state.value.value()[k] + job->value[k] > route.courier->value[k]) {
            flag = false;
            break;
        }
    }
    return flag;
}

optional<answer_t> MadrichEngine::insert_job(const ptrJob &job, const ptrStorage &storage) {
    int a, b, c;
    a = b = c = -1;
    State best_state;

    for (int i = 0; i < routes.size(); ++i) {
        Route &route = routes[i];
        if (!check_route(route) ||
            !RvrpProblem::validate_storage(storage, route.courier) ||
            !RvrpProblem::validate_skills(job, route.courier) ||
            !RvrpProblem::validate_skills(storage, route.courier)) {
            continue;
        }

        for (int j = 0; j < route.tracks.size(); ++j) {
            Track &track = route.tracks[j];
            if (track.storage != storage || !check_value(job, track, route)) {
                continue;
            }

            Jobs tmp = track.jobs;
            for (int k = 0; k < track.jobs.size(); ++k) {
                std::vector new_jobs = insert(k, job, track.jobs);
                track.jobs = new_jobs;
                std::optional state = RvrpProblem::get_state(route);
                if (state && (a == -1 || state.value() - route.state < best_state)) {
                    best_state = state.value() - route.state;  // все так же ищем лучшее
                    a = i;
                    b = j;
                    c = k;
                }
                track.jobs = tmp;
            }
        }
    }

    if (a == -1) {
        return std::nullopt;
    }
    return std::make_tuple(best_state, a, b, c);
}

optional<answer_t> MadrichEngine::insert_track(const ptrJob &job, const ptrStorage &storage) {
    int a, b;
    a = b = -1;
    State best_state;

    for (int i = 0; i < routes.size(); ++i) {
        Route &route = routes[i];
        if (!check_route(route) ||
            !RvrpProblem::validate_storage(storage, route.courier) ||
            !RvrpProblem::validate_skills(job, route.courier) ||
            !RvrpProblem::validate_skills(storage, route.courier)) {
            continue;
        }

        Track track(job, storage);
        State min_dt = RvrpProblem::get_state_track(track, route);
        if (!RvrpProblem::validate_courier(route.state + min_dt, route)) {
            continue;
        }

        if (route.tracks.empty()) {  // если в маршруте вообще ничего еще нет
            route.tracks.push_back(track);
            std::optional state = RvrpProblem::get_state(route);
            if (state && (a == -1 || state.value() - route.state < best_state)) {
                best_state = state.value() - route.state;
                a = i;
                b = 0;
            }
            route.tracks.pop_back();
        } else {  // если есть, перебираем места вставки
            for (int j = 0; j < route.tracks.size(); ++j) {
                route.tracks.insert(route.tracks.begin() + j, track);
                std::optional state = RvrpProblem::get_state(route);
                if (state && (a == -1 || state.value() - route.state < best_state)) {
                    best_state = state.value() - route.state;
                    a = i;
                    b = j;
                }
                route.tracks.erase(route.tracks.begin() + j);
            }
        }
    }

    if (a == -1) {
        return std::nullopt;
    }
    return std::make_tuple(best_state, a, b, 0);
}
