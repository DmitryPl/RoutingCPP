#include "engine.h"
#include <generators.h>


void replace_job(int job_id, Track &track) {
    // replace to unassigned jobs
    track.storage->unassigned_jobs.push_back(track.jobs[job_id]);
    track.jobs.erase(track.jobs.begin() + job_id);
}

void MadrichEngine::random_ruin(uint32_t number) {
    uint32_t s = assigned_jobs();
    number = number > s ? s : number;  // ну мы не можем удалить больше точек, чем есть вообще

    for (int i = 0; i < number; ++i) {
        while (true) {
            if (routes.empty()) {
                break;  // тогда точно удалять нечего
            }
            int route_id = generate_number(routes.size());
            Route &route = routes[route_id];
            if (route.tracks.empty()) {
                continue;
            }

            int track_id = generate_number(route.tracks.size());
            Track &track = route.tracks[track_id];
            auto size = track.jobs.size();
            if (size == 0) {
                remove_empty_tracks();  // лучше удалить, вдруг там все пустые
                continue;
            }

            replace_job(generate_number(size), track);
            mark_route(true, route);
            break;
        }
    }
    
    remove_empty_tracks();
}

[[maybe_unused]] void MadrichEngine::radial_ruin(uint32_t radius) {
    int route_id, track_id, job_id;  // TODO: check function + mark_routes
    while (true) {
        if (routes.empty()) {
            return;  // тогда точно удалять нечего
        }
        route_id = generate_number(routes.size());
        track_id = generate_number(routes[route_id].tracks.size());
        auto size = routes[route_id].tracks[track_id].jobs.size();
        if (size == 0) {
            remove_empty_tracks();  // лучше удалить, вдруг там все пустые
            continue;
        }
        job_id = generate_number(size);
        break;
    }

    int matrix_id = routes[route_id].tracks[track_id].jobs[job_id]->location.matrix_id;
    Track &tr = routes[route_id].tracks[track_id];  // не забываем переместить
    replace_job(job_id, tr);

    for (auto &route: routes) {
        for (auto &track : route.tracks) {  // перемещаем, а потом удаляем, если попал в радиус
            track.jobs.erase(std::remove_if(track.jobs.begin(), track.jobs.end(),
                                            [&matrix_id, &track, &route, &radius](const ptrJob &job) {
                                                int id = job->location.matrix_id;
                                                if (route.matrix.get_time(matrix_id, id) > radius) {
                                                    track.storage->unassigned_jobs.push_back(job);
                                                    return true;
                                                }
                                                return false;
                                            }), track.jobs.end());
        }
    }
}
