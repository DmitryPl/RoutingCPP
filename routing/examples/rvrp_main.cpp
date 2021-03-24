#include <generators.h>
#include <local_search/problem.h>


int main() {
    printf("Generating...\n");
    auto[vec, couriers, storages, matrix] = generate_rvrp(50, 3, 2);
    printf("Building...\n");
    MadrichEngine tour = RvrpProblem::init_tour(vec, storages, couriers, matrix, true);
    tour.improve(60, 5, 0, false, false);
}
