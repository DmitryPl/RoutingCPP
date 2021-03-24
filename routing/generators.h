#ifndef MADRICH_SOLVER_GENERATORS_H
#define MADRICH_SOLVER_GENERATORS_H

#include <vector>
#include <random>
#include <map>
#include <cmath>
#include "base_model.h"

/**
 * @return Рандомное значение от 0 до 1
 */
float generate_value();

/**
 * @return random bool
 */
bool generate_bool();

/**
 * @return random int [0, max)
 */
int generate_number(int max);

/**
 * @return Два рандомных значения от 0 до 1
 */
std::tuple<float, float> generate_tuple();

/**
 * Генерация рандомных точек в формате lat, lon в заданном квадрате
 * @param n кол-во точек
 * @param min_x
 * @param max_x
 * @param min_y
 * @param max_y
 * @return список из n таких точек
 */
std::vector<std::tuple<float, float>>
generate_points(int n, float min_x = 55.65, float max_x = 55.82, float min_y = 37.45, float max_y = 37.75);

/**
 * Генерация матрицы расстояния для набора точек в формате lat, lon
 * @param points список точек в (lat, lon)
 * @return matrix
 */
std::vector<std::vector<int>> generate_distance(const std::vector<std::tuple<float, float>> &points);

/**
 * Генерация матрицы времени для набора точек в формате lat, lon
 * @param points список точек в (lat, lon)
 * @return matrix
 */
std::vector<std::vector<time_t>> generate_time(const std::vector<std::tuple<float, float>> &points);

/**
 * Создание заказов из набора точек
 * @param points список точек
 * @param start с какого индекса начать в массиве
 * @param end на какоом индексе закончить (не включительно)
 * @param storage_id
 * @return список задач
 */
Jobs generate_jobs(const std::vector<Point> &points, int start, int end, const std::string &storage_id);

/**
 * Генерация задачи с несколькими складами
 * @param jobs кол-во задач на склад
 * @param storages кол-во складов
 * @param couriers кол-во курьеров
 * @return сгенерированные сущности
 */
std::tuple<int, Couriers, Storages, std::map<std::string, Matrix>>
generate_rvrp(int jobs, int storages, int couriers);

#endif //MADRICH_SOLVER_GENERATORS_H
