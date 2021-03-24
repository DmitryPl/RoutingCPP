#ifndef MADRICH_SOLVER_PROBLEM_H
#define MADRICH_SOLVER_PROBLEM_H

#include <base_model.h>
#include <local_search/engine.h>


/**
 * Есть какие-то стандартные оптимизации, переборы ребер тура
 * Заранее пока мы не можем после оптимизации с уверенностью сказать, что маршрут норм
 * Также мы не можем просто после оптимизации оценить цену, стоимость маршрута
 * Поэтому после оптимизации проще перезапустить валидацию и оценки
 * (хотя все таки довольно дорого по времени, так что лучше заранее отбрасывать точно плохие варианты)
 *
 * Еще тут построение тура жадным методом:
 * Курьер выезжает из точки, выбирает ближайший склад, на котором есть хотя бы один не развезенный заказ (новый склад
 * всегда выбирается так), который он сможет отвезти, учитывая ограничения. Если таких несколько (заказов),
 * то выбирается меньший по State (все следующие заказы выбираем так). Далее выбирается следующий "ближайший" заказ,
 * который он посетит, учитывая ограничения. Так он их развозит пока это возможно, после он либо выбирает
 * ближайший другой склад, либо едет на тот же склад, либо едет на конечную точку своего маршрута
 */


/**
 * Создание тура, валиадция, оценка и пр.
 */
class RvrpProblem {
public:
    /**
     * Создание тура
     * @param vec размер вектора вместимости
     * @param storages все склады
     * @param couriers все доступные курьеры
     * @param matrices матрицы для профилей курьеров
     * @param circle_track обязан ли курьер возвращаться на склад
     * @return новый тур
     */
    static MadrichEngine init_tour(
            uint16_t vec,
            Storages &storages,
            Couriers &couriers,
            std::map<std::string, Matrix> &matrices,
            bool circle_track
    );

    /**
     * Создание маршрута
     * @param vec размер вектора вместимости
     * @param courier курьер для этого маршрута
     * @param matrices матрицы
     * @param circle_track обязан ли курьер возвращаться на склад
     * @return новый маршрут
     */
    static Route init_route(
            uint16_t vec,
            ptrCourier &courier,
            std::map<std::string, Matrix> &matrices,
            bool circle_track
    );

    /**
     * Создание подмаршрута 
     * @param current_point текущее положение
     * @param state текущая оценка маршрута
     * @param route маршрут
     * @return новое состояние, подмаршрут с одной задачей
     */
    static std::optional<std::tuple<State, Track>>
    init_track(int current_point, const State &state, Route &route);

    /**
     * Оценка и валидация всего маршрута
     * @param route маршрут
     * @return состояние
     */
    static std::optional<State> get_state(const Route &route);

    /**
     * Оценка стоимости подмаршрута, без ожиданий и предыдущих грехов
     * @param track подмаршрут
     * @param route маршрут
     * @return оценка
     */
    static State get_state_track(const Track &track, const Route &route);

    /**
     * Курьеру хватит умений доставить заказ
     */
    static bool validate_skills(const ptrJob &job, const ptrCourier &courier);

    /**
     * Курьеру хватит умений работать с этим складом
     */
    static bool validate_skills(const ptrStorage &storage, const ptrCourier &courier);

    /**
     * На данный момент маршрут не нарушает ограничений курьера
     */
    static bool validate_courier(const State &state, const Route &route);

    /**
     * Может заезжать на этот склад
     */
    static bool validate_storage(const ptrStorage &storage, const ptrCourier &courier);

private:
    /**
     * Выбор следующей задачи
     * @param location текущая позиция
     * @param state текущая оценка маршрута
     * @param track подмаршрут текущий
     * @param route маршрут
     * @return новое состояние, если найдена задача
     */
    static std::optional<State> choose_job(int location, const State &state, Track &track, Route &route);

    /**
     * Возвращает отсортированные склады по дальности
     * @param curr_point текущая позиция
     * @param state текущая оценка маршрута
     * @param route маршрут
     * @return время доезда, индекс в общем списке
     */
    static std::vector<std::tuple<time_t, std::size_t>>
    sorted_storages(int curr_point, const State &state, const Route &route);

    /**
     * Оценка стоимости поездка на заказ
     * @param curr_point текущее положение курьера
     * @param state текущее состояние всего тура
     * @param job заказ
     * @param storage с какого склада
     * @param route маршрут
     * @return стоимость без включения предыдущей части
     */
    static std::optional<State>
    go_job(int curr_point, const State &state, const ptrJob &job, const ptrStorage &storage, const Route &route);

    /**
     * Оценка стоимости поездки на склад
     * @param curr_point текущее положение курьера
     * @param state текущее состояние тура
     * @param storage склад
     * @param route маршрут
     * @return стоимость без включений предыдущего
     */
    static std::optional<State>
    go_storage(int curr_point, const State &state, const ptrStorage &storage, const Route &route);

    /**
     * Стоимость за определенный промежуток
     * @param travel_time время в секундах
     * @param distance расстояние в метрах
     * @param route маршрут
     * @return стоимость
     */
    static float cost(time_t travel_time, int distance, const Route &route);

    /**
     * Сколько секунд придется подождать курьеру, чтобы попасть в ближайшее временное окно
     * @param arrival_time время в туре (arrival + start = current)
     * @param start_time время начала тура с создания мира
     * @param time_windows временные окна
     * @return ожидание в секундах (-1, если не возможно попасть)
     */
    static time_t waiting(time_t arrival_time, time_t start_time, const std::vector<Window> &time_windows);

    /**
     * Сколько секунд придется подождать курьеру, чтобы попасть во временное окно
     * @param arrival_time время путешествия в туре (arrival + start = current)
     * @param start_time время начала тура с создания мира
     * @param time_window временное окно
     * @return ожидание в секундах (-1, если не возможно попасть)
     */
    static time_t waiting(time_t arrival_time, time_t start_time, const Window &time_window);

    /**
     * Оценка поездки с текущий точки на конечную точку
     * @param curr_point текущее положение курьера
     * @param state оценка тура всего
     * @param route маршрут
     * @return оценка без включения предыдущего
     */
    static std::optional<State> end(int curr_point, const State &state, const Route &route);
};

#endif //MADRICH_SOLVER_PROBLEM_H
