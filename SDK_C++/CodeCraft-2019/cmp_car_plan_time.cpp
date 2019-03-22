#include "cmp_car_plan_time.h"
#include "car.h"

bool cmp_car_plan_time::operator() (const car &a, const car &b ){
        return a.get_plan_time() < b.get_plan_time();
}