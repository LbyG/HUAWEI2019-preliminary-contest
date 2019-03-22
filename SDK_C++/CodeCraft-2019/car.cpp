#include "car.h"

#include <vector>
#include <iostream>

using namespace std;

car::car() {
    // TODO
    this->id = 0;
}

car::car(string car_info) {
    // car_info = (id,from,to,speed,planTime)
    int len = car_info.size();
    vector<int> info_val;
    info_val.clear();
    int previous_char_pos = -1;
    int val = 0;
    for (int i = 0; i < len; ++i) {
        if (car_info[i] < '0' || car_info[i] > '9') {
            if (i > (previous_char_pos + 1)) {
                info_val.push_back(val);
                val = 0;
            }
            previous_char_pos = i;
        } else {
            val = val * 10 + (int(car_info[i]) - int('0'));
        }
    }
    this->id = info_val[0];
    this->from = info_val[1];
    this->to = info_val[2];
    this->speed = info_val[3];
    this->plan_time = info_val[4];
}

// return car id
int car::get_id() const {
    return this->id;
}

// reutrn car start from cross_id;
int car::get_from() const {
    return this->from;
}

// return car arrive to cross_id;
int car::get_to() const {
    return this->to;
}

// return car speed
int car::get_speed() const {
    return this->speed;
}

// return car plan start time
int car::get_plan_time() const {
    return this->plan_time;
}

// set this -> schedule_status = schedule_status
void car::set_schedule_status(int schedule_status) {
    this->schedule_status = schedule_status;
}

// return car schedule status
int car::get_schedule_status() const {
    return this->schedule_status;
}