#ifndef MADRICH_SOLVER_BASE_MODEL_H
#define MADRICH_SOLVER_BASE_MODEL_H

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <optional>
#include <iomanip>
#include <utility>
#include <sstream>
#include <memory>

using std::shared_ptr;


/**
 * Тур, который описывает решение задачи, - список маршрутов (Routes),
 * Каждый маршрут состоит из треков или петель (подмаршрут, Track) и привязан к определенному курьеру
 * Трек - некоторые задачи (или заказы, Job), связанные с определенным складом
 * т.е. либо петля склад - задачи - склад, либо просто трек склад А - задачи - склад Б
 * Тура, как отдельной сущности, в итоге не существует - это просто массив Маршрутов в MadrichEngine
 *
 * Курьер перемешается по определенной матрице времени и расстояний, матрица задана профилем, все дано извне
 * У куреьера есть точки старта, конца, какие-то дополнительные ограницений, не всегда может посещаться все склады
 * У каждого Склада есть задачи, которые закреплены за ним, их нельзя передать другому складу,
 * но движок как бы передает их Курьеру для развоза, нужно развести как можно больше заказов
 */


/**
 * Временное окно
 */
class Window {
public:
    std::tuple<time_t, time_t> window;  // начало окно - конец окна

    explicit Window() = default;

    Window(const Window &window) = default;

    explicit Window(std::tuple<time_t, time_t> window);

    explicit Window(const std::string &start_t, const std::string &end_t);

    [[maybe_unused]] void print() const;
};


/**
 * Точка на карте
 */
class Point {
public:
    int matrix_id = -1;  // индекс в матрице расстояния/времени
    std::tuple<float, float> point; // (lat, lon)

    explicit Point() = default;

    Point(const Point &point) = default;

    explicit Point(int matrix_id, std::tuple<float, float> point);

    [[maybe_unused]] void print() const;
};


/**
 * Стоимость работы курьера
 */
class Cost {
public:
    float start = 0;  // за старт фиксированная
    float second = 0;  // за секунду работы
    float meter = 0;  // за метр работы

    explicit Cost() = default;

    Cost(const Cost &cost) = default;

    explicit Cost(float start, float second, float meter);

    [[maybe_unused]] void print() const;
};


/**
 * Сущность для хранения матриц
 */
class Matrix {
private:
    std::vector<std::vector<std::vector<int>>> distance;  // матрица матриц расстояний
    std::vector<std::vector<std::vector<time_t>>> travel_time;  // матрица матриц времени
    uint32_t discreteness = 15;  // дискретность матриц (все время разбито по 15 минут по дефолту)
    time_t start_time = 0;  // время начала первой матрицы (начало периода, в котором мы отслеживаем матрицы)
    time_t end_time = 0;  // время конца акутальности матрицы (конец периода, в котором мы отслеживаем матрицы)

public:
    std::string profile;  // профиль матрицы (водитель, пешеход, велосипедист...)

    explicit Matrix() = default;

    Matrix(const Matrix &matrix) = default;

    explicit Matrix(std::string profile,
                    std::vector<std::vector<int>> distance,
                    std::vector<std::vector<time_t>> travel_time);

    explicit Matrix(std::string profile,
                    std::vector<std::vector<std::vector<int>>> distance,
                    std::vector<std::vector<std::vector<time_t>>> travel_time,
                    uint32_t discreteness,
                    time_t start_time,
                    time_t end_time);

    [[nodiscard]] time_t get_time(uint32_t src, uint32_t dst, time_t curr_time = 0) const;

    [[nodiscard]] int get_distance(uint32_t src, uint32_t dst, time_t curr_time = 0) const;

    [[maybe_unused]] void print() const;
};


/**
 * Некоторая оценка, цена, стоимость маршрута, тура или куска чего-то
 */
class State {
public:
    time_t travel_time = 0;  // время
    int distance = 0;  // расстояние
    float cost = 0;  // стоимость
    std::optional<std::vector<int>> value;  // вектор загруженности

    explicit State() = default;

    State(const State &state) = default;

    explicit State(time_t travel_time, int distance, float cost, std::optional<std::vector<int>> value = std::nullopt);

    [[maybe_unused]] void print() const;

    State operator+(const State &rhs) const;

    State operator-(const State &rhs) const;

    State &operator+=(const State &rhs);

    State &operator-=(const State &rhs);

    bool operator<(const State &rhs) const;

private:
    static std::optional<std::vector<int>> sum_values(const State &lt, const State &rt);
};


/**
 * Заказ
 */
class Job {
public:
    int delay = 0;  // время на обслуживание
    int priority = 0;  // приоритет
    std::string job_id;  // имя; unique
    std::vector<int> value;  // вектор веса/объема
    std::vector<std::string> skills;  // требуемые умения
    Point location = Point(-1, {0, 0});  // точка на карте; not unique
    std::vector<Window> time_windows;  // временные окна для доставки

    Job() = default;

    Job(int delay,
        std::string job_id,
        std::vector<int> value,
        std::vector<std::string> skills,
        const Point &location,
        std::vector<Window> time_windows);

    Job(int delay,
        int priority,
        std::string job_id,
        std::vector<int> value,
        std::vector<std::string> skills,
        const Point &location,
        std::vector<Window> time_windows);

    [[maybe_unused]] void print() const;

    bool operator==(const Job &other) const;
};

typedef shared_ptr<Job> ptrJob;
typedef std::vector<shared_ptr<Job>> Jobs;


/**
 * Склад
 */
class Storage {
public:
    int load = 0;  // время на обслуживание
    std::string name;  // название
    std::vector<std::string> skills;  // требуемые умения
    Point location;  // точка на карте
    Window work_time;  // время раобты
    Jobs unassigned_jobs;  // неназначенные еще задачи для этого склада

    explicit Storage() = default;

    explicit Storage(
            int load,
            std::string name,
            std::vector<std::string> skills,
            const Point &location,
            const Window &work_time,
            Jobs unassigned_jobs
    );

    explicit Storage(
            int load,
            std::string name,
            std::vector<std::string> skills,
            const Point &location,
            const Window &work_time
    );

    [[maybe_unused]] void print() const;
};

typedef shared_ptr<Storage> ptrStorage;
typedef std::vector<shared_ptr<Storage>> Storages;


/**
 * Курьер
 */
class Courier {
public:
    std::string name;  // имя
    std::string profile;  // профиль матрицы
    Cost cost;  // стоимость работы
    std::vector<int> value;  // вектор вместимости
    std::vector<std::string> skills;  // умения
    int max_distance = 0;  // максимально проезжаемая дистанция (если равно нулю, не учитывается)
    Window work_time;  // время смены
    Point start_location;  // точка старта
    Point end_location;  // точка конца
    [[maybe_unused]] Storages storages;  // доступные склады

    explicit Courier() = default;

    Courier(
            std::string name,
            std::string profile,
            const Cost &cost,
            std::vector<int> value,
            std::vector<std::string> skills,
            int max_distance,
            const Window &work_time,
            const Point &start_location,
            const Point &end_location
    );

    Courier(
            std::string name,
            std::string profile,
            const Cost &cost,
            std::vector<int> value,
            std::vector<std::string> skills,
            int max_distance,
            const Window &work_time,
            const Point &start_location,
            const Point &end_location,
            Storages storages
    );

    [[maybe_unused]] void print() const;

    bool operator==(const Courier &other) const;
};

typedef shared_ptr<Courier> ptrCourier;
typedef std::vector<shared_ptr<Courier>> Couriers;


/**
 * Подмаршрут
 */
class Track {
public:
    ptrStorage storage;  // задачи берутся из этого склада
    Jobs jobs;  // назначенные задачи

    Track() = default;

    explicit Track(ptrStorage storage) : storage(std::move(storage)) {}

    explicit Track(const ptrJob &job, ptrStorage storage) : storage(std::move(storage)), jobs({job}) {}

    [[maybe_unused]] void print() const;
};


/**
 * Маршрут
 */
class Route {
public:
    uint16_t vec = 0;  // размерность вектора вместимости
    ptrCourier courier;  // курьер
    Matrix matrix;  // Матрица курьера
    time_t start_time = 0;  // время начала с начала мира
    State state;  // стоимость маршрута
    bool circle_track = true;  // надо возвращаться на склад
    std::vector<Track> tracks;  // все подмаршруты

    explicit Route() = default;

    Route(uint16_t vec, time_t start_time, bool circle_track, ptrCourier courier, const Matrix &matrix);

    /**
     * Кол-во задач, назначенных на этого курьера
     */
    [[nodiscard]] std::size_t assigned_jobs() const;

    /**
     * Колв-о доступных задач, неназначенных на этого курьера
     */
    [[nodiscard]] std::size_t unassigned_jobs() const;

    [[maybe_unused]] void print() const;

    [[maybe_unused]] void draw() const;
};


#endif //MADRICH_SOLVER_BASE_MODEL_H
