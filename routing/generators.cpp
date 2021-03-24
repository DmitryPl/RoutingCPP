#include "generators.h"


#define earthRadiusKm 6371.0

double deg2rad(double deg) {
    return (deg * 3.14 / 180);
}

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in meters
 */
double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg2rad(lat1d);
    lon1r = deg2rad(lon1d);
    lat2r = deg2rad(lat2d);
    lon2r = deg2rad(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);
    return earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v)) * 2e3;
}

float generate_value() {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    return static_cast<float>(mersenne()) / static_cast<float>(UINT32_MAX);
}

bool generate_bool() {
    std::mt19937_64 rng((std::random_device()) ());
    std::uniform_int_distribution<int> uni(0, 1);
    return bool(uni(rng));
}

int generate_number(int max) {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<int> uni(0, max - 1);
    return uni(rng);
}

std::tuple<float, float> generate_tuple() {
    return {generate_value(), generate_value()};
}

std::vector<std::tuple<float, float>> generate_points(int n, float min_x, float max_x, float min_y, float max_y) {
    float diff_x = max_x - min_x;
    float diff_y = max_y - min_y;
    std::vector ret = std::vector<std::tuple<float, float>>(n);
    for (auto &point : ret) {
        auto[x, y] = generate_tuple();
        x = x * diff_x + min_x;
        y = y * diff_y + min_y;
        point = std::tuple(x, y);
    }
    return ret;
}

std::vector<std::vector<int>> generate_distance(const std::vector<std::tuple<float, float>> &points) {
    std::size_t size = points.size();
    std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            auto&[first_x, first_y] = points[i];
            auto&[second_x, second_y] = points[j];
            double value = distanceEarth(first_x, first_y, second_x, second_y);
            matrix[i][j] = value;
            matrix[j][i] = value;
        }
    }
    return matrix;
}

std::vector<std::vector<time_t>> generate_time(const std::vector<std::tuple<float, float>> &points) {
    std::size_t size = points.size();
    std::vector<std::vector<time_t>> matrix(size, std::vector<time_t>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            auto&[first_x, first_y] = points[i];
            auto&[second_x, second_y] = points[j];
            double value = distanceEarth(first_x, first_y, second_x, second_y);
            matrix[i][j] = value / 10;
            matrix[j][i] = value / 10;
        }
    }
    return matrix;
}

Jobs generate_jobs(const std::vector<Point> &points, int start, int end, const std::string &storage_id) {
    int size = end - start;
    Jobs jobs(size);
    for (int i = 0; i < size; ++i) {
        std::ostringstream start_t;
        std::ostringstream end_t;
        start_t << "2020-10-01T" << 10 + (i % 4) << ":00:00Z";
        end_t << "2020-10-01T" << 12 + (i % 4) << ":00:00Z";
        Window window = Window(start_t.str(), end_t.str());
        jobs[i] = std::make_shared<Job>(Job(
                300,
                storage_id + "_" + std::to_string(i),
                {1, 2},
                {"brains"},
                points[i + start],
                {window})
        );
    }
    return jobs;
}

std::tuple<int, Couriers, Storages, std::map<std::string, Matrix>>
generate_rvrp(int jobs, int storages, int couriers) {
    int size = jobs * storages + storages + couriers;
    std::vector pts = generate_points(size);
    std::vector distance = generate_distance(pts);
    std::vector travel_time = generate_time(pts);
    Matrix matrix("driver", distance, travel_time);
    std::map<std::string, Matrix> ret_matrix = {{"driver", matrix}};
    std::vector<Point> points(size);
    for (int i = 0; i < size; ++i) {
        points[i] = Point(i, pts[i]);
    }

    Window window("2020-10-01T10:00:00Z", "2020-10-01T20:00:00Z");
    Storages storage_list(storages);
    for (int i = 0; i < storages; ++i) {
        std::string storage_id = "storage_" + std::to_string(i);
        storage_list[i] = std::make_shared<Storage>(Storage(
                300,
                storage_id,
                {"brains"},
                points[storages * jobs + i],
                window,
                generate_jobs(points, i * jobs, (i + 1) * jobs, storage_id))
        );
    }

    Couriers courier_list(couriers);
    for (int i = 0; i < couriers; ++i) {
        Point &courier_loc = points[jobs * storages + storages + i];
        std::string name = "courier_" + std::to_string(i);
        courier_list[i] = std::make_shared<Courier>(Courier(
                name,
                "driver",
                Cost(10., 0.5, 1.2),
                {40, 80},
                {"brains"},
                0,
                window,
                courier_loc,
                courier_loc,
                storage_list)
        );
    }
    printf("Generated\n\n");
    return {2, courier_list, storage_list, ret_matrix};
}
