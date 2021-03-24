#include <generators.h>
#include <local_search/problem.h>


int main() {
    printf("Generating...\n");
    auto[vec, couriers, storages, matrix] = generate_rvrp(150, 1, 10);
    printf("Building...\n");
    MadrichEngine tour = RvrpProblem::init_tour(vec, storages, couriers, matrix, true);

    tour.improve();
    for (const auto &route : tour.routes) {
        route.print();
    }
}
