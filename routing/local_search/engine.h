#ifndef MADRICH_SOLVER_ENGINE_H
#define MADRICH_SOLVER_ENGINE_H

#include <base_model.h>
#include <utility>
#include <set>
#include <map>
#include <chrono>

using namespace std::chrono;
typedef std::optional<time_point<system_clock>> optional_end;

/**
 * Оптимизация
 * Оптимизация на данный момент происходит в три этапа: ruin, recreate, local search.
 * Local search включает в себя операторы оптимизации и переодическую попытку вставки доп. заказов в тур
 * Ruin: эвристика, которая по некоторым правилам удаляет из тура некоторые точки
 * Recreate: эвристика, которая по некоторым правилам (сейчас это вставка обычная), докидывает точки в тур
 * Когда этап заканчивается, если критерии остановки не наступили, запускается оптимизация заново
 * (Перед повторяющимися перезапусками, один раз запускается local search)
 *
 * Постоптимизация
 * Включается если 1. разрешена в принципе 2. если на прошлой итерации не удалось улучшиться вообще
 *
 * Вставки
 * Выбираем все заказы, максимального приоритета (0), пытаемся лучшим образом вставить их в тур (оцениваем по State)
 * После того, как все возможные вставки закончились, понижаем приоритет, пытаемся вставить их. Вообще есть 2 вставки:
 * либо вставляем в уже существующий подмаршрут, либо создаем подмаршрут в маршруте, с учетом текущих правил.
 * Вставками можно тур с нуля построить, но пока это довольно медленно, хотя тур лучше,
 * чем жадным методом и можно учесть приоритеты задач.
 */


/**
 * Движок поиска
 */
class MadrichEngine {
public:
    bool ignore_priority = true;  // игнорируем ли приоритеты задачи
    Storages storages;  // все склады в задаче
    std::vector<Route> routes;  // все маршруты для курьеров

    explicit MadrichEngine() = default;

    explicit MadrichEngine(uint16_t vec,
                      Storages &storages,
                      Couriers &couriers,
                      std::map<std::string, Matrix> &matrices,
                      bool circle_track,
                      bool ignore_priority);

    explicit MadrichEngine(Storages storages, uint32_t size, bool ignore_priority = true);

    /**
     * Построения тура через вставки
     * Долго, но качетсвенно + приоритеты учтет
     */
    void build_tour();

    /**
     * Улучшить тур!
     * @param work_time время на улучшение (0: не ограничивать)
     * @param max_fails сколько раз за полную фазу оптимизаций не получить тур лучше лучшего
     * @param phases сколько фаз улучшения доступно (0: не прерываться)
     * @param post_three_opt использовать для пост-оптимизации 3-opt
     * @param post_cross использовать для пост-оптимизации Cross-exchange
     */
    void improve(uint32_t work_time = 0,
                 uint32_t max_fails = 5,
                 uint32_t phases = 0,
                 bool post_three_opt = false,
                 bool post_cross = false);

    /**
     * Одну задачу; Если склада нет в списке, то задача не будет добавлена
     */
    [[maybe_unused]] void add_job(ptrJob &job, ptrStorage &storage);

    /**
     * Много задач; Если склада нет в списке, то задача не будет добавлена
     */
    [[maybe_unused]] void add_jobs(Jobs &jobs, ptrStorage &storage);

    /**
     * Одну задачу
     */
    [[maybe_unused]] void remove_job(ptrJob &job, ptrStorage &storage);

    /**
     * Много задач
     */
    [[maybe_unused]] void remove_jobs(Jobs &jobs, ptrStorage &storage);

    /**
     * Оценка всего тура
     */
    [[nodiscard]] State get_state() const;

    /**
     * Кол-во неназначенных заказов в этой проблеме
     */
    [[nodiscard]] std::size_t unassigned_jobs() const;

    /**
     * Кол-во назначенных задач для этой проблемы
     */
    [[nodiscard]] std::size_t assigned_jobs() const;

    [[maybe_unused]] void print() const;

    [[maybe_unused]] void draw() const;

private:
    //// Block section

    /**
     * Не кончились ли еще доступные фазы?
     * @param phases доступные фазы
     * @return продолжаем?
     */
    [[nodiscard]] bool check_continue(uint32_t phases, optional_end end) const;

    // Блокировка тура, если на текущей и на предыдущей фазе не удалось улучшить маршрут
    // Значит для текущих операторов он уже оптимален
    uint32_t phase = 0;  // текущая фаза/итерация улучшения
    std::map<std::string, bool> previous_phase;  // удалось ли улучшить маршрут для курьера на пред. фазе
    std::map<std::string, bool> current_phase;  // удалось ли улучшить маршрут для курьера на тек. фазе
    std::set<std::tuple<uint32_t, uint32_t, time_t, int, float>> taboo_set;  // assigned, unassigned, time, distance, cost

    /**
     * Проверка акутальности словаря, запустить перед improve
     * Если маршрут не найден в блокировках curr=true, prev=false
     */
    void check_block();

    /**
     * Выставляем curr=true, prev=false, taboo.clear()
     */
    void set_zeros();

    /**
     * Смена фазы на новую (prev=curr, curr=false)
     */
    void update_phase();

    /**
     * Отмечаем удалось ли улучшить маршрут
     * @param value да/нет
     * @param route
     */
    void mark_route(bool value, const Route &route);

    /**
     * Отмечаем удалось ли улучшить оба маршрута
     * @param value да/нет
     * @param route1
     * @param route2
     */
    void mark_route(bool value, const Route &route1, const Route &route2);

    /**
     * Проверяем попытаться ли оптитимизировать маршрут (менялся на прошлой или на текущей фазе)
     * @param route
     * @return заблокирован или нет
     */
    bool check_route(const Route &route);

    //// Improve section

    /**
     * Последовательный запуск local_search / ruin / recreate
     * Перезапускаемся пока не кончатся фазы, время или 5 раз не сможем улучшить результат
     * @param max_fails сколько раз за полную фазу оптимизаций не получить тур лучше лучшего
     * @param phases сколько фаз улучшения доступно (0: не прерываться)
     * @param post_three_opt использовать для пост-оптимизации 3-opt
     * @param post_cross использовать для пост-оптимизации Cross-exchange
     * @param end момент прерывания оптимизации
     */
    void continuous_improve(uint32_t max_fails,
                            uint32_t phases,
                            bool post_three_opt,
                            bool post_cross,
                            optional_end end);

    /**
     * Прогон всех локальных оптимизаций
     * @param phases сколько фаз улучшения доступно (0: не прерываться)
     * @param post_three_opt использовать для пост-оптимизации 3-opt
     * @param post_cross использовать для пост-оптимизации Cross-exchange
     * @param end момент прерывания оптимизации
     */
    void improve_tour(uint32_t phases, bool post_three_opt, bool post_cross, optional_end end);

    /**
     * Оптимизация через inter операторы
     * @param post_cross использовать ли cross
     * @param end точка остановки улучшений
     * @return получилось ли улучшить тур
     */
    bool inter_improve(bool post_cross, optional_end end);

    /**
     * Inter-improve для двух маршрутов
     * @param i первый
     * @param j второй
     * @param post_cross использовать ли
     * @param end точка остановки улучшений
     * @return получилось ли улучшить
     */
    bool improve_double(uint32_t i, uint32_t j, bool post_cross, optional_end end);

    /**
     * Inter-improve для одного маршрута
     * @param i номер маршрута
     * @param post_cross использовать ли
     * @param end точка остановки улучшений
     * @return получилось ли улучшить
     */
    bool improve_one(uint32_t i, bool post_cross, optional_end end);

    /**
     * Оптимизация через intra операторы
     * @param post_three_opt использовать ли 3-opt
     * @param end точка остановки улучшений
     * @return получилось ли улучшить тур
     */
    bool intra_improve(bool post_three_opt, optional_end end);

    /**
     * Убираем пустые треки из маршрутов
     */
    void remove_empty_tracks();

    //// Insert Section

    /**
     * Вставка неназначенных еще задач
     * @param priority игнорировать ли приоритеты
     * @return получилось ли вставить
     */
    bool unassigned_insert();

    /**
     * Ищем лучшую вставку с учетом текущего приоритета
     * @param current_priority
     * @return получилось вставить или нет
     */
    bool insert_best(uint32_t current_priority);

    /**
     * Выбираем лучшую вставку из вставки задачи в трек и трек в маршрут
     * @param best_state текущее лучшее delta state
     * @param job задача
     * @param storage откуда заказ
     * @return каким методом, delta state + куда
     */
    std::optional<std::tuple<char, std::tuple<State, int, int, int>>>
    choose_best(const std::optional<State> &best_state, const ptrJob &job, const ptrStorage &storage);

    /**
     * Вставить в текущие подмаршруты
     * @param storage откуда заказ
     * @param job сам заказ
     * @return delta state, куда
     */
    std::optional<std::tuple<State, int, int, int>> insert_job(const ptrJob &job, const ptrStorage &storage);

    /**
     * Создает для вставки подмаршрут с новой точкой
     * @param storage откуда заказ
     * @param job сам заказ
     * @return delta state, куда
     */
    std::optional<std::tuple<State, int, int, int>> insert_track(const ptrJob &job, const ptrStorage &storage);

    /**
     * Максимальный приоритет неназначенных задач
     */
    [[nodiscard]] uint32_t max_priority() const;

    /**
     * Generate tuple for taboo hash
     */
    [[nodiscard]] std::tuple<uint32_t, uint32_t, time_t, int, float> generate_hash() const;

    /**
     * Save hash in taboo set
     */
    void save_tour();

    /**
     * Проверяем, что такой тур мы еще не получали
     * @param delta разница между предыдущим и текущим состоянием
     * @return принимать ли новое состояние
     */
    bool check_tour(const State &delta);

    /**
     * Меняем в оригинале tracks и state, если табу разрешает
     * Лучше или хуже мы не проверяем
     * @param route оригинал
     * @param route_copy копия
     * @return поменяли ли
     */
    bool get_from_copy(Route& route, Route& route_copy);

    /**
     * Меняем в оригинале tracks и state, если табу разрешает
     * Лучше или хуже мы не проверяем
     * @param route1 копия
     * @param route1_copy оригинал
     * @param route2 копия
     * @param route2_copy оригинал
     * @return поменяли ли
     */
    bool get_from_copy(Route& route1, Route& route1_copy, Route& route2, Route& route2_copy);

    //// Ruin Section

    /**
     * Выкидывает рандомные number точек из тура
     */
    void random_ruin(uint32_t number);

    [[maybe_unused]] void radial_ruin(uint32_t radius);
};

#endif //MADRICH_SOLVER_ENGINE_H
