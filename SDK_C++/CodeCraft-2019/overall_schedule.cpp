#include "overall_schedule.h"

#include <iostream>

using namespace std;

// return cars number
int overall_schedule::get_cars_n() {
    return cars.size();
}

// function to clear priority queue
void overall_schedule::clear_priority_queue(priority_queue<car, vector<car>, cmp_car_plan_time> &pq) {
    while (! pq.empty()) {
        pq.pop();
    }
}

//load cars, roads and crosses info from car_path, road_path and cross_path file
void overall_schedule::load_cars_road_cross(string car_path, string road_path, string cross_path) {
    // TODO
}

// load cars' path from answer_path
void overall_schedule::load_answer(string answer_path) {
    // TODO
}

// Initial all running cars schedule status, car.schedule_status = 0
void overall_schedule::initial_cars_schedule_status() {
    // TODO
}

// Schedule cars in road.
// If car can through cross then car into schedule wait -> car.schedule_status = 1
// If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
// If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
// If car blocked by end state car then car move to the back of the previous car
void overall_schedule::schedule_cars_in_road() {
    // TODO
}

int overall_schedule::schedule_cars() {
    int T = 0;
    // initial state
    cars_wait_plan_time_n = get_cars_n(); // number of cars which T < car.plan_time
    clear_priority_queue(cars_wait_plan_time_list); // cars which T < car.plan_time
    for (map<int, car>::iterator iter = cars.begin(); iter != cars.end(); ++iter)
        cars_wait_plan_time_list.push(iter->second);
    cars_wait_run_n = 0; // the number of cars wait to start run
    clear_priority_queue(cars_wait_plan_time_list); // cars which T < car.plan_time
    cars_running_n = 0; // the number of cars is running
    car_wait_in_cross_n = 0; // the number of cars wait to through cross or previous car is wait to through cross
    /*
    // If all cars is arrive then break
    while (cars_wait_plan_time_n > 0 || cars_wait_run_n > 0 || cars_running_n > 0) {
        // Initial all running cars schedule status, car.schedule_status = 0
        initial_cars_schedule_status();
        // Schedule cars in road.
        // If car can through cross then car into schedule wait -> car.schedule_status = 1
        // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
        // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
        // If car blocked by end state car then car move to the back of the previous car
        schedule_cars_in_road();
        // Cycling the relevant roads at each cross until all car are end state
        while (car_wait_in_cross_n > 0) {
            for (all crosses) {
                // todo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                schedule_cars_in_cross()
            }
        }
        // Schedule cars which wait start
        schedule_cars_wait_start();
        T ++;
    }
    */
    return T;
}