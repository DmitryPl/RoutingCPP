#include "route_utils.h"


Jobs swap(const Jobs &jobs, uint32_t x, uint32_t y) {
    std::vector new_jobs = std::vector(jobs);
    uint32_t size = jobs.size();

    uint32_t temp = 0;
    if (x < y) {
        temp = (y - x + 1) / 2;
    } else if (x > y) {
        temp = ((size - x) + y + 2) / 2;
    }

    for (uint32_t i = 0; i < temp; ++i) {
        std::swap(new_jobs[(x + i) % size], new_jobs[(y - i) % size]);
    }

    return new_jobs;
}

Jobs three_opt_exchange(const Jobs &jobs, uint32_t best_exchange, uint32_t x, uint32_t y, uint32_t z) {
    uint32_t size = jobs.size();

    uint32_t b = (x + 1) % size;
    uint32_t c = y % size;
    uint32_t d = (y + 1) % size;
    uint32_t e = z % size;

    if (best_exchange == 0) {
        return swap(swap(jobs, b, e), b, b + (e - d));
    } else if (best_exchange == 1) {
        return swap(swap(swap(jobs, b, e), b, b + (e - d)), e - (c - b), e);
    } else if (best_exchange == 2) {
        return swap(swap(jobs, b, e), e - (c - b), e);
    } else if (best_exchange == 3) {
        return swap(swap(jobs, d, e), b, c);
    } else if (best_exchange == 4) {
        return swap(jobs, b, e);
    } else if (best_exchange == 5) {
        return swap(jobs, d, e);
    } else if (best_exchange == 6) {
        return swap(jobs, b, c);
    } else {
        printf("Unexpected error!\n");
        return jobs;
        // throw std::invalid_argument("Bad exchange for three opt");
    }
}

std::tuple<Jobs, Jobs> cross(const Jobs &jobs1, const Jobs &jobs2, uint32_t it1, uint32_t it2, uint32_t it3, uint32_t it4) {
    uint32_t size1 = jobs1.size();
    uint32_t size2 = jobs2.size();
    uint32_t new_size1 = size1 - (it2 - it1) + (it4 - it3);
    uint32_t new_size2 = size2 - (it4 - it3) + (it2 - it1);
    Jobs new_jobs1(new_size1);
    Jobs new_jobs2(new_size2);

    for (uint32_t i = 0; i < it1; ++i) {
        new_jobs1[i] = jobs1[i];
    }
    for (uint32_t i = 0; i < it4 - it3 + 1; ++i) {
        new_jobs1[i + it1] = jobs2[i + it3];
    }
    for (uint32_t i = 1; i < size1 - it2; ++i) {
        new_jobs1[i + it1 + (it4 - it3)] = jobs1[i + it2];
    }
    for (uint32_t i = 0; i < it3; i++) {
        new_jobs2[i] = jobs2[i];
    }
    for (uint32_t i = 0; i < it2 - it1 + 1; ++i) {
        new_jobs2[i + it3] = jobs1[i + it1];
    }
    for (uint32_t i = 1; i < size2 - it4; ++i) {
        new_jobs2[i + it3 + (it2 - it1)] = jobs2[i + it4];
    }

    return {new_jobs1, new_jobs2};
}

std::tuple<Jobs, Jobs> replace_point(const Jobs &jobs1, const Jobs &jobs2, uint32_t it1, uint32_t it2) {
    uint32_t size1 = jobs1.size();
    uint32_t size2 = jobs2.size();
    Jobs new_jobs1(size1 + 1);
    Jobs new_jobs2(size2 - 1);

    for (uint32_t i = 0; i < size1 + 1; ++i) {
        if (i == it1) {
            new_jobs1[i] = jobs2[it2];
        } else if (i < it1) {
            new_jobs1[i] = jobs1[i];
        } else {
            new_jobs1[i] = jobs1[i - 1];
        }
    }

    for (uint32_t i = 0; i < size2 - 1; ++i) {
        if (i < it2) {
            new_jobs2[i] = jobs2[i];
        } else {
            new_jobs2[i] = jobs2[i + 1];
        }
    }

    return {new_jobs1, new_jobs2};
}

Jobs insert(uint32_t place, const ptrJob &job, const Jobs &jobs) {
    uint32_t size = jobs.size() + 1;
    Jobs new_route = Jobs(size);
    for (uint32_t i = 0; i < size; ++i) {
        if (i < place) {
            new_route[i] = jobs[i];
        } else if (i == place) {
            new_route[i] = job;
        } else {
            new_route[i] = jobs[i - 1];
        }
    }
    return new_route;
}
