#include "car.h"

car::car(string car_info) {
    // TODO
}

// return car id
int car::get_id() const {
    return id;
}

// reutrn car start from cross_id;
int car::get_from() const {
    return from;
}

// return car arrive to cross_id;
int car::get_to() const {
    return to;
}

// return car speed
int car::get_speed() const {
    return speed;
}

// return car plan start time
int car::get_plan_time() const {
    return plan_time;
}

// set this -> schedule_status = schedule_status
void car::set_schedule_status(int schedule_status) {
    this->schedule_status = schedule_status;
}

// return car schedule status
int car::get_schedule_status() const {
    return schedule_status;
}