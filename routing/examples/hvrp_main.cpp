#include <iostream>
#include <generators.h>
#include <local_search/problem.h>


int main() {
    printf("Generating...\n");
    auto[vec, couriers, storages, matrix] = generate_rvrp(70, 3, 5);
    printf("Building...\n");
    MadrichEngine tour = MadrichEngine(vec, storages, couriers, matrix, true, true);
    tour.build_tour();

    for (const auto &route : tour.routes) {
        route.state.print();
        auto state = RvrpProblem::get_state(route);
        if (state) {
            state.value().print();
        } else {
            printf("failed\n");
        }
        printf("\n");
    }

    tour.improve(10, 5, 0, false, false);
    auto state = tour.get_state();
    state.print();
}
