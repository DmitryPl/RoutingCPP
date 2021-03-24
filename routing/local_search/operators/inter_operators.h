#ifndef MADRICH_SOLVER_INTER_OPERATORS_H
#define MADRICH_SOLVER_INTER_OPERATORS_H

#include <local_search/operators/route_utils.h>
#include <local_search/engine.h>
#include <local_search/problem.h>


/**
 * Обмен двух точек между маршрутами
 * @param track1 подмаршрут в
 * @param route1 первом маршруте
 * @param track2 подмаршрут во
 * @param route2 втором маршруте
 * @param end остановка расчета
 * @return улучшилась ли их сумма
 */
bool inter_swap(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end);

/**
 * Перемещение точки из одного в другой
 * @param track1 подмаршрут в
 * @param route1 первом маршруте
 * @param track2 подмаршрут во
 * @param route2 втором маршруте
 * @param end остановка расчета
 * @return улучшилась ли их сумма
 */
bool inter_replace(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end);

/**
 * Кусков подмаршрутов
 * @param track1 подмаршрут в
 * @param route1 первом маршруте
 * @param track2 подмаршрут во
 * @param route2 втором маршруте
 * @param end остановка расчета
 * @return улучшилась ли их сумма
 */
bool inter_cross(Track &track1, Route &route1, Track &track2, Route &route2, optional_end end);

#endif //MADRICH_SOLVER_INTER_OPERATORS_H
