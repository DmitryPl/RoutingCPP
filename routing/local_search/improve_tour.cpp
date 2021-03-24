#include "engine.h"

#include <local_search/operators/inter_operators.h>
#include <local_search/operators/intra_operators.h>
#include <generators.h>


void MadrichEngine::improve(
        uint32_t work_time,
        uint32_t max_fails,
        uint32_t phases,
        bool post_three_opt,
        bool post_cross
) {
    printf("Improve started\n");
    auto start_t = system_clock::now();
    optional_end stop_moment = std::nullopt;
    if (work_time != 0) {
        stop_moment = start_t + seconds(work_time);
    }
    check_block();
    draw();

    continuous_improve(max_fails, phases, post_three_opt, post_cross, stop_moment);

    draw();
    printf("\nDone, %jd seconds\n\n", duration_cast<seconds>(system_clock::now() - start_t).count());
}

void MadrichEngine::continuous_improve(
        uint32_t max_fails,
        uint32_t phases,
        bool post_three_opt,
        bool post_cross,
        optional_end end
) {
    std::vector best_routes(routes);  // Будем хранить лучшую копию до конца улучшений
    State best_state(get_state());  // есть идеи получше?
    uint32_t best_jobs = assigned_jobs();
    uint32_t fail = 0;

    while (fail < max_fails && check_continue(phases, end)) {
        printf("\nBest; jobs: %ju, tt: %jd, cost: %f\n",
               assigned_jobs(), best_state.travel_time, best_state.cost);

        improve_tour(phases, post_three_opt, post_cross, end);  // запускаем оптимизацию
        State new_state = get_state();
        uint32_t new_jobs = assigned_jobs();
        if (new_jobs > best_jobs || (new_state < best_state && new_jobs >= best_jobs)) {
            printf("Tour Improved!\n");
            best_state = new_state;
            best_jobs = new_jobs;
            best_routes = std::vector(routes);
            fail = 0;
        } else {
            fail++;
        }

        if (!(fail < max_fails && check_continue(phases, end))) {
            break;  // если все равно вылетаем, то не ломаем ничего
        }

        routes = std::vector(best_routes);
        uint32_t num = assigned_jobs();  // будем удалять от 5% до 15% точек
        float delta = ((float(num) / float(6.67)) - (float(num) / 20)) / max_fails;  // вычисляем шаг
        num = num / 10 + uint32_t(delta * fail);  // вычисляем, сколько удалим сейчас
        num = num == 0 ? 5 : num;  // там меньше 10 задач... ну пусть 5 удалит хоть
        random_ruin(num);  // ruin
        unassigned_insert();  // recreate
    }

    routes = best_routes;
}

void MadrichEngine::improve_tour(uint32_t phases, bool post_three_opt, bool post_cross, optional_end end) {
    bool changed = true;
    bool post_intra = false;
    bool post_inter = false;
    bool last_hope = post_three_opt or post_cross;

    while ((changed or last_hope) && check_continue(phases, end)) {
        if (!changed && last_hope) {  // вот тогда уже врубаем все пост оптимизации
            post_intra = post_three_opt;
            post_inter = post_cross;
        }

        changed = false;
        bool vr = generate_bool();

        // inter и intra могут запускаться в разном порядке,
        // что в теории может приводить к разным локальным минимумам
        // постоптимизации включаются только, если вообще не удалось улучшить на пред ходу
        if (vr && intra_improve(post_intra, end)) {
            changed = true;
            if (end && end.value() < system_clock::now()) { break; }
        }
        if (inter_improve(post_inter, end)) {
            changed = true;
            if (end && end.value() < system_clock::now()) { break; }
        }
        if (!vr && intra_improve(post_intra, end)) {
            changed = true;
            if (end && end.value() < system_clock::now()) { break; }
        }
        if (unassigned_insert()) {
            changed = true;
            if (end && end.value() < system_clock::now()) { break; }
        }
        update_phase();

        if ((post_inter || post_intra)) {  // значит запускалась пост оптимизация
            post_intra = false;
            post_inter = false;
            last_hope = changed;  // восстанавливаем, если оптимизировалось
        }
    }
}

bool MadrichEngine::check_continue(uint32_t phases, optional_end end) const {
    return (!end || end.value() >= system_clock::now()) && (phases == 0 || phase + 1 < phases);
}

bool MadrichEngine::improve_double(uint32_t i, uint32_t j, bool post_cross, optional_end end) {
    bool result = false;
    Route &route1 = routes[i];
    Route &route2 = routes[j];
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    Route route1_copy = route1;  // работаем с копиями
    Route route2_copy = route2;
    for (uint32_t k = 0; k < route1_copy.tracks.size(); ++k) {
        for (uint32_t l = 0; l < route2_copy.tracks.size(); ++l) {
            if (!post_cross && (!check_route(route1) && !check_route(route2))) {
                printf("\nBLOCKED\n");
                continue;
            }
            Track &track1 = route1_copy.tracks[k];
            Track &track2 = route2_copy.tracks[l];

            if (!post_cross && inter_swap(track1, route1_copy, track2, route2_copy, end)) {
                result = get_from_copy(route1, route1_copy, route2, route2_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
            if (!post_cross && inter_replace(track1, route1_copy, track2, route2_copy, end)) {
                result = get_from_copy(route1, route1_copy, route2, route2_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
            if (post_cross && inter_cross(track1, route1_copy, track2, route2_copy, end)) {
                result = get_from_copy(route1, route1_copy, route2, route2_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
        }
    }

    mark_route(result, route1, route2);
    return result;
}

bool MadrichEngine::improve_one(uint32_t i, bool post_cross, optional_end end) {
    bool result = false;
    Route &route = routes[i];
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    Route route_copy = route;  // работаем с копией
    for (uint32_t k = 0; k < route_copy.tracks.size(); ++k) {
        for (uint32_t l = k + 1; l < route_copy.tracks.size(); ++l) {
            if (!post_cross && !check_route(route)) {
                printf("\nBLOCKED\n");
                continue;
            }
            Track &track1 = route_copy.tracks[k];
            Track &track2 = route_copy.tracks[l];

            if (!post_cross && inter_swap(track1, route_copy, track2, route_copy, end)) {
                result = get_from_copy(route, route_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
            if (!post_cross && inter_replace(track1, route_copy, track2, route_copy, end)) {
                result = get_from_copy(route, route_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
            if (post_cross && inter_cross(track1, route_copy, track2, route_copy, end)) {
                result = get_from_copy(route, route_copy);
                if (end && end.value() < system_clock::now()) { break; }
            }
        }
    }

    mark_route(result, route);
    return result;
}

bool MadrichEngine::inter_improve(bool post_cross, optional_end end) {
    bool result = false;
    auto size = routes.size();
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    for (uint32_t i = 0; i < size; ++i) {
        for (uint32_t j = i; j < size; ++j) {
            if (i != j) {
                if (improve_double(i, j, post_cross, end)) {
                    result = true;
                }
            } else {  // ну значит улучшение между треками в маршруте
                if (improve_one(i, post_cross, end)) {
                    result = true;
                }
            }
        }
    }

    remove_empty_tracks();
    return result;
}

bool three_opt(Route &route, optional_end end) {
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    bool changed = false;
    for (auto &track : route.tracks) {
        if (three_opt(track, route, end)) {
            changed = true;
        }
    }
    return changed;
}

bool two_opt(Route &route, optional_end end) {
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    bool changed = false;
    for (auto &track : route.tracks) {
        if (two_opt(track, route, end)) {
            changed = true;
        }
    }
    return changed;
}

bool MadrichEngine::intra_improve(bool post_three_opt, optional_end end) {
    bool result = false;
    if (end && end.value() < system_clock::now()) {
        return false;
    }

    for (auto &route : routes) {
        if (end && end.value() < system_clock::now()) {
            break;
        }

        if (!post_three_opt && !check_route(route)) {
            printf("\nBLOCKED\n");
            continue;
        }

        bool changed = false;
        bool status;
        Route route_copy = route;  // меняем только копию
        if (!post_three_opt) {
            status = two_opt(route_copy, end);
        } else {
            status = three_opt(route_copy, end);
        }

        if (status) {
            result = changed = get_from_copy(route, route_copy);  // если она нас устраивает, меняем
        }
        mark_route(changed, route);
    }

    return result;
}

void MadrichEngine::remove_empty_tracks() {
    for (auto &route : routes) {
        route.tracks.erase(
                std::remove_if(
                        route.tracks.begin(),
                        route.tracks.end(),
                        [](const Track &track) { return track.jobs.empty(); }
                ),
                route.tracks.end()
        );
    }
}
