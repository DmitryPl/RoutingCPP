#include "inter_operators.h"


bool inter_swap(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end) {
    if (track1.storage != track2.storage) {
        return false;
    }

    uint32_t size1 = track1.jobs.size();
    uint32_t size2 = track2.jobs.size();
    State state1 = route1.state;
    State state2 = route2.state;
    State state = route1.state + route2.state;
    bool changed = true;
    bool result = false;
    printf("\nSwap started, tt: %jd, cost: %f\n", state.travel_time, state.cost);

    while (changed) {
        changed = false;
        State best_state1, best_state2;
        State best_state = state;
        uint32_t a, b;

        for (uint32_t it1 = 0; it1 < size1; ++it1) {
            for (uint32_t it2 = 0; it2 < size2; ++it2) {
                std::swap(track1.jobs[it1], track2.jobs[it2]);  // TODO: check opt
                std::optional new_state1 = RvrpProblem::get_state(route1);
                std::optional new_state2 = RvrpProblem::get_state(route2);
                if ((!new_state1) || (!new_state2)) {
                    std::swap(track1.jobs[it1], track2.jobs[it2]);
                    continue;
                }
                State new_state = new_state1.value() + new_state2.value();
                if (new_state < best_state) {
                    result = changed = true;
                    best_state = new_state;
                    best_state1 = new_state1.value();
                    best_state2 = new_state2.value();
                    a = it1;
                    b = it2;
                }
                std::swap(track1.jobs[it1], track2.jobs[it2]);
            }
        }

        if (changed) {
            state = best_state;
            state1 = best_state1;
            state2 = best_state2;
            std::swap(track1.jobs[a], track2.jobs[b]);
            if (end && end.value() < system_clock::now()) { changed = false; }
            printf("Updated, tt: %jd, cost: %f\n", best_state.travel_time, best_state.cost);
        }
    }
    printf("Ended, tt: %jd, cost %f\n", state.travel_time, state.cost);
    if (result) {
        route1.state = state1;
        route2.state = state2;
    }
    return result;
}

std::optional<std::tuple<State, State>>
get_states(std::tuple<Jobs, Jobs> &new_jobs, Track &track1, Route &route1, Track &track2, Route &route2) {
    auto&[new_jobs1, new_jobs2] = new_jobs;
    std::vector old_jobs1 = track1.jobs;
    std::vector old_jobs2 = track2.jobs;

    track1.jobs = new_jobs1;
    track2.jobs = new_jobs2;
    std::optional new_state1 = RvrpProblem::get_state(route1);
    std::optional new_state2 = RvrpProblem::get_state(route2);
    track1.jobs = old_jobs1;
    track2.jobs = old_jobs2;

    if (new_state1 && new_state2) {
        return std::make_tuple(new_state1.value(), new_state2.value());
    }
    return std::nullopt;
}

bool uns_inter_replace(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end) {
    std::vector jobs1 = track1.jobs;
    std::vector jobs2 = track2.jobs;
    State state1 = route1.state;
    State state2 = route2.state;
    State state = route1.state + route2.state;
    bool changed = true;
    bool result = false;

    while (changed) {
        changed = false;
        State best_state1, best_state2;
        Jobs best_jobs1, best_jobs2;
        State best_state = state;
        uint32_t size1 = track1.jobs.size();
        uint32_t size2 = track2.jobs.size();

        for (uint32_t it1 = 0; it1 < size1; ++it1) {
            for (uint32_t it2 = 0; it2 < size2; ++it2) {
                std::tuple new_jobs = replace_point(track1.jobs, track2.jobs, it1, it2);  // TODO: remove bad opt
                std::optional answer = get_states(new_jobs, track1, route1, track2, route2);
                if (!answer) {  // а еще трек вообще может быть убран из маршрута
                    continue;
                }

                auto&[new_state1, new_state2] = answer.value();
                State new_state = new_state1 + new_state2;
                if (new_state < best_state) {
                    result = changed = true;
                    best_state = new_state;
                    best_state1 = new_state1;
                    best_state2 = new_state2;
                    auto&[new_jobs1, new_jobs2] = new_jobs;
                    best_jobs1 = new_jobs1;
                    best_jobs2 = new_jobs2;
                }
            }
        }
        if (changed) {
            state = best_state;
            state1 = best_state1;
            state2 = best_state2;
            track1.jobs = best_jobs1;
            track2.jobs = best_jobs2;
            if (end && end.value() < system_clock::now()) { changed = false; }
            printf("Updated, tt: %jd, cost: %f\n", best_state.travel_time, best_state.cost);
        }
    }
    if (result) {
        route1.state = state1;
        route2.state = state2;
    }
    return result;
}

bool inter_replace(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end) {
    if (track1.storage != track2.storage) {
        return false;
    }

    bool result = false;
    bool changed = true;
    bool changed1, changed2;
    State state = route1.state + route2.state;
    printf("\nReplace started, tt: %jd, cost: %f\n", state.travel_time, state.cost);

    while (changed) {
        changed1 = uns_inter_replace(track1, route1, track2, route2, end);
        if (end && end.value() < system_clock::now()) { break; }
        changed2 = uns_inter_replace(track2, route2, track1, route1, end);
        if (end && end.value() < system_clock::now()) { break; }
        changed = changed1 | changed2;
        if (changed) {
            result = changed;
        }
    }

    state = route1.state + route2.state;
    printf("Ended, tt: %jd, cost %f\n", state.travel_time, state.cost);
    return result;
}

bool inter_cross(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end) {
    if (track1.storage != track2.storage) {
        return false;
    }

    uint32_t size1 = track1.jobs.size();
    uint32_t size2 = track2.jobs.size();
    std::vector jobs1 = track1.jobs;
    std::vector jobs2 = track2.jobs;
    State state = route1.state + route2.state;
    printf("\nCross started, tt: %jd, cost: %f\n", state.travel_time, state.cost);

    for (uint32_t it1 = 0; it1 < size1; ++it1) {
        for (uint32_t it2 = it1; it2 < size1; ++it2) {
            for (uint32_t it3 = 0; it3 < size2; ++it3) {
                for (uint32_t it4 = it3; it4 < size2; ++it4) {
                    std::tuple new_jobs = cross(jobs1, jobs2, it1, it2, it3, it4);  // TODO: remove bad opt
                    std::optional answer = get_states(new_jobs, track1, route1, track2, route2);
                    if (!answer) {  // а еще трек вообще может быть убран из маршрута
                        continue;
                    }

                    auto&[new_state1, new_state2] = answer.value();
                    State new_state = new_state1 + new_state2;
                    if (new_state < state) {
                        auto&[new_jobs1, new_jobs2] = new_jobs;
                        track1.jobs = new_jobs1;
                        track2.jobs = new_jobs2;
                        route1.state = new_state1;
                        route2.state = new_state2;
                        printf("Updated, tt: %jd, cost: %f\n", new_state.travel_time, new_state.cost);
                        return true;
                    }

                    if (end && end.value() < system_clock::now()) {
                        printf("Ended\n");
                        return false;
                    }
                }
            }
        }
    }
    printf("Ended\n");
    return false;
}
