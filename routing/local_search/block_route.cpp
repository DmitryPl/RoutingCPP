#include "engine.h"


void MadrichEngine::update_phase() {
    phase += 1;
    save_tour();
    previous_phase = current_phase;
    for (const auto &route : current_phase) {
        current_phase[route.first] = false; // в новой фазе еще ничего не поменялось
    }
}

bool MadrichEngine::check_route(const Route &route) {
    return previous_phase[route.courier->name] || current_phase[route.courier->name];
}

void MadrichEngine::mark_route(bool value, const Route &route) {
    bool current_state = current_phase[route.courier->name];
    if (!current_state) {
        current_phase[route.courier->name] = value;
    }
}

void MadrichEngine::mark_route(bool value, const Route &route1, const Route &route2) {
    mark_route(value, route1);
    mark_route(value, route2);
}

void MadrichEngine::check_block() {
    for (const auto &route : routes) {
        if (!current_phase.contains(route.courier->name)) {
            current_phase[route.courier->name] = false;
        }
        if (!previous_phase.contains(route.courier->name)) {
            previous_phase[route.courier->name] = true;
        }
    }
}

void MadrichEngine::set_zeros() {
    check_block();
    taboo_set.clear();
    for (const auto &route : routes) {
        current_phase[route.courier->name] = false;
        previous_phase[route.courier->name] = true;
    }
}
