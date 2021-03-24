#ifndef MADRICH_SOLVER_VRP_UTILS_H
#define MADRICH_SOLVER_VRP_UTILS_H

#include <stdexcept>
#include <base_model.h>


/**
 * Вставка задачи в массив из других задач
 * @param place на какое место должен встать
 * @param job какая задача
 * @param jobs массив задач
 * @return массив задач со вставленной задачей
 */
Jobs insert(uint32_t place, const ptrJob &job, const Jobs &jobs);

/**
 * Разворот куска массива задач в списке задач
 * @param jobs массив задач
 * @param x от включительно
 * @param y до включительно
 * @return новый массив задач
 */
Jobs swap(const Jobs &jobs, uint32_t x, uint32_t y);

/**
 * Изменения массива задач для 3-opt оптимизации
 * @param jobs массив задач
 * @param best_exchange номер оптимизации (0-6)
 * @param x
 * @param y
 * @param z
 * @return новый массив задач
 */
Jobs three_opt_exchange(const Jobs &jobs, uint32_t best_exchange, uint32_t x, uint32_t y, uint32_t z);

/**
 * Cross-exchange для двух массивов задач
 * @param jobs1 первый массив
 * @param jobs2 второй массив
 * @param it1 от в первом
 * @param it2 до в первом
 * @param it3 от во втором
 * @param it4 до во втором
 * @return новые измененные массивы
 */
std::tuple<Jobs, Jobs> cross(const Jobs &jobs1, const Jobs &jobs2, uint32_t it1, uint32_t it2, uint32_t it3, uint32_t it4);

/**
 * Перемещение точки из одного массива в другой (2 -> 1)
 * @param jobs1 массив куда произойдет вставка
 * @param jobs2 массив откуда удалим точку
 * @param it1 куда вставить
 * @param it2 откуда вытащить
 * @return новые массивы
 */
std::tuple<Jobs, Jobs> replace_point(const Jobs &jobs1, const Jobs &jobs2, uint32_t it1, uint32_t it2);

#endif //MADRICH_SOLVER_VRP_UTILS_H
