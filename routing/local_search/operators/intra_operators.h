#ifndef MADRICH_SOLVER_INTRA_OPERATORS_H
#define MADRICH_SOLVER_INTRA_OPERATORS_H

#include <local_search/operators/route_utils.h>
#include <local_search/engine.h>
#include <local_search/problem.h>


/**
 * Перебор по три ребра
 * @param track подмаршрут
 * @param route маршрут
 * @param end остановка расчета
 * @return улучшился или нет
 */
bool three_opt(Track &track, Route &route, optional_end end);

/**
 * Перебор по два ребра
 * @param track подмаршрут
 * @param route маршрут
 * @param end остановка расчета
 * @return улучшился или нет
 */
bool two_opt(Track &track, Route &route, optional_end end);

#endif //MADRICH_SOLVER_INTRA_OPERATORS_H
