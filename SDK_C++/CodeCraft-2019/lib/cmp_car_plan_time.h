#ifndef CMP_CAR_PLAN_TIME
#define CMP_CAR_PLAN_TIME

#include "car.h"

struct cmp_car_plan_time {
public:
    bool operator() (const car &a, const car &b );
};

#endif