#include "intra_operators.h"


bool three_opt(Track &track, Route &route, optional_end end) {
    State tmp_state = route.state;
    Jobs tmp_jobs = track.jobs;
    uint32_t size = track.jobs.size();
    bool changed = true;
    printf("\nThree opt started, tt: %jd, cost: %f\n", tmp_state.travel_time, tmp_state.cost);

    while (changed) {
        changed = false;
        State best_state = tmp_state;
        std::vector best_jobs = tmp_jobs;

        for (uint32_t it1 = 0; it1 < size; ++it1) {
            for (uint32_t it3 = it1 + 1; it3 < size; ++it3) {
                for (uint32_t it5 = it3 + 1; it5 < size; ++it5) {
                    for (uint32_t i = 0; i < 4; ++i) {
                        Jobs new_jobs = three_opt_exchange(tmp_jobs, i, it1, it3, it5);
                        track.jobs = new_jobs;
                        std::optional new_state = RvrpProblem::get_state(route);
                        if (!new_state) {
                            continue;
                        }
                        if (new_state.value() < best_state) {
                            changed = true;
                            best_state = new_state.value();
                            best_jobs = new_jobs;
                        }
                        track.jobs = tmp_jobs;
                    }
                }
            }
        }
        if (changed) {
            tmp_state = best_state;
            tmp_jobs = best_jobs;
            if (end && end.value() < system_clock::now()) { changed = false; }
            printf("Updated, tt: %jd, cost: %f\n", best_state.travel_time, best_state.cost);
        }
    }
    printf("Ended, tt: %jd, cost %f\n", tmp_state.travel_time, tmp_state.cost);
    track.jobs = tmp_jobs;
    if (tmp_state < route.state) {
        route.state = tmp_state;
        return true;
    }
    return false;
}


bool two_opt(Track &track, Route &route, optional_end end) {
    State tmp_state = route.state;
    Jobs tmp_jobs = track.jobs;
    uint32_t size = track.jobs.size();
    bool changed = true;
    printf("\nTwo opt started, tt: %jd, cost: %f\n", tmp_state.travel_time, tmp_state.cost);

    while (changed) {
        changed = false;
        State best_state = tmp_state;
        std::vector best_jobs = tmp_jobs;

        for (uint32_t it1 = 0; it1 < size; ++it1) {
            for (uint32_t it3 = it1 + 1; it3 < size; ++it3) {
                Jobs new_jobs = swap(tmp_jobs, it1, it3);
                track.jobs = new_jobs;
                std::optional new_state = RvrpProblem::get_state(route);
                if (!new_state) {
                    continue;
                }
                if (new_state.value() < best_state) {
                    changed = true;
                    best_state = new_state.value();
                    best_jobs = new_jobs;
                }
                track.jobs = tmp_jobs;
            }
        }
        if (changed) {
            tmp_state = best_state;
            tmp_jobs = best_jobs;
            if (end && end.value() < system_clock::now()) { changed = false; }
            printf("Updated, tt: %jd, cost: %f\n", best_state.travel_time, best_state.cost);
        }
    }
    printf("Ended, tt: %jd, cost %f\n", tmp_state.travel_time, tmp_state.cost);
    track.jobs = tmp_jobs;
    if (tmp_state < route.state) {
        route.state = tmp_state;
        return true;
    }
    return false;
}
