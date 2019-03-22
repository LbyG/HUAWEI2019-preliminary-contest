#include "overall_schedule.h"

#include <iostream>
#include <fstream>

using namespace std;

// return cars number
int overall_schedule::get_cars_n() {
    return this->cars.size();
}

// function to clear priority queue
void overall_schedule::clear_priority_queue(priority_queue<car, vector<car>, cmp_car_plan_time> &pq) {
    while (! pq.empty()) {
        pq.pop();
    }
}

// function to clear priority queue
void overall_schedule::clear_priority_queue(priority_queue<car, vector<car>, cmp_car_id> &pq) {
    while (! pq.empty()) {
        pq.pop();
    }
}

//load cars, roads and crosses info from car_path, road_path and cross_path file
void overall_schedule::load_cars_roads_crosses(string car_path, string road_path, string cross_path) {
    // load cars information from car_path file
    this->cars.clear();
    ifstream cars_info_file(car_path);
    string car_info;
    if (cars_info_file)
    {
        while (getline(cars_info_file, car_info)) {
            if (car_info.size() > 0 && car_info[0] != '#') {
                car new_car = car(car_info);
                this->cars[new_car.get_id()] = new_car;
            }
        }
    }
    cars_info_file.close();
    // load roads information from road_path file
    this->roads.clear();
    ifstream roads_info_file(road_path);
    string road_info;
    if (roads_info_file)
    {
        while (getline(roads_info_file, road_info)) {
            if (road_info.size() > 0 && road_info[0] != '#') {
                road new_road = road(road_info);
                this->roads[new_road.get_id()] = new_road;
            }
        }
    }
    roads_info_file.close();
    // load crosses information from cross_path file
    this->crosses.clear();
    ifstream crosses_info_file(cross_path);
    string cross_info;
    if (crosses_info_file)
    {
        while (getline(crosses_info_file, cross_info)) {
            if (cross_info.size() > 0 && cross_info[0] != '#') {
                cross new_cross = cross(cross_info);
                this->crosses[new_cross.get_id()] = new_cross;
            }
        }
    }
    roads_info_file.close();
    // connect road info and cross info
    for (map<int, road>::iterator iter = this->roads.begin(); iter != this->roads.end(); ++iter) {
        road from_to_road = iter->second;
        crosses[iter->second.get_to()].add_road_into_cross(&from_to_road);
        crosses[iter->second.get_from()].add_road_departure_cross(iter->first, &from_to_road);
        if (iter->second.get_is_duplex() == 1) {
            road to_from_road = iter->second;
            crosses[iter->second.get_from()].add_road_into_cross(&to_from_road);
            crosses[iter->second.get_to()].add_road_departure_cross(iter->first, &to_from_road);
        }
    }
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
    this->cars_wait_plan_time_n = get_cars_n(); // number of cars which T < car.plan_time
    this->clear_priority_queue(this->cars_wait_plan_time_list); // cars which T < car.plan_time
    for (map<int, car>::iterator iter = this->cars.begin(); iter != this->cars.end(); ++iter) {
        this->cars_wait_plan_time_list.push(iter->second);
    }
    this->cars_wait_run_n = 0; // the number of cars wait to start run
    this->clear_priority_queue(this->cars_wait_run_list); // cars which T < car.plan_time
    this->cars_running_n = 0; // the number of cars is running
    this->car_wait_in_cross_n = 0; // the number of cars wait to through cross or previous car is wait to through cross
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