#include "overall_schedule.h"

#include "util.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// return cars number
int overall_schedule::get_cars_n() {
    return this->cars.size();
}

//load cars, roads and crosses info from car_path, road_path and cross_path file
void overall_schedule::load_cars_roads_crosses(string car_path, string road_path, string cross_path) {
    // load cars information from car_path file
    // car_info = (id,from,to,speed,planTime)
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
    // road_info = (id,length,speed,channel,from,to,isDuplex)
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
    // cross_info = (id,roadId1,roadId2,roadId3,roadId4)
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
    // because iter->first is id, so road add to  road_into_cross is from small to large by road_id;
    this->roads_connect_cross.clear();
    for (map<int, road>::iterator iter = this->roads.begin(); iter != this->roads.end(); ++iter) {
        // road from from_id to to_id connect to crosses
        this->roads_connect_cross.push_back(road(iter->second));
        road* from_to_road = &this->roads_connect_cross.back();
        this->crosses[iter->second.get_to()].add_road_into_cross(from_to_road);
        this->crosses[iter->second.get_from()].add_road_departure_cross(iter->first, from_to_road);
        // if isDuplex == 1m road from to_id to from_id connect to crosses
        if (iter->second.get_is_duplex() == 1) {
            this->roads_connect_cross.push_back(road(iter->second));
            this->roads_connect_cross.back().swap_from_to();
            road* to_from_road = &this->roads_connect_cross.back();
            this->crosses[iter->second.get_to()].add_road_into_cross(to_from_road);
            this->crosses[iter->second.get_from()].add_road_departure_cross(iter->first, to_from_road);
        }
    }
}

// load cars' schedule plan from answer_path
// schedule_info = [id, schedule_start_time, schedule_path1, schedule_path2, schedule_path3, schedule_path4, ...]
void overall_schedule::load_answer(string answer_path) {
    ifstream answer_info_file(answer_path);
    string answer_info;
    if (answer_info_file)
    {
        while (getline(answer_info_file, answer_info)) {
            if (answer_info.size() > 0 && answer_info[0] != '#') {
                vector<int> answer_vec = parse_string_to_int_vector(answer_info);
                cars[answer_vec[0]].set_schedule_path(answer_vec);
            }
        }
    }
    answer_info_file.close();
}

// initial all data in T = 0
void overall_schedule::initial_cars_state_in_T0() {
    // number of cars which T < car.schedule_start_time
    this->cars_wait_schedule_start_time_n = this->get_cars_n(); 
    // cars which T < car.schedule_start_time, priority depend on schedule_start_time
    clear_priority_queue(this->cars_wait_schedule_start_time_list); 
    for (map<int, car>::iterator iter = this->cars.begin(); iter != this->cars.end(); ++iter) {
        this->cars_wait_schedule_start_time_list.push(iter->second);
    }
    // the number of cars which T >= car.schedule_start_time but wait to running in road
    this->cars_wait_run_n = 0; 
    // cars which T >= car.schedule_start_time but wait to running in road, priority depend on id
    clear_priority_queue(this->cars_wait_run_list); 
    // the number of cars which is running in road
    this->cars_running_n = 0; 
    // the number of cars which is arrive destination
    this->cars_arrive_destination_n = 0;
    
    // the number of cars wait to through cross or previous car is wait to through cross
    this->cars_running_wait_state_n = 0; 
    // the number of cars was run in this time unit
    this->cars_running_termination_state_n = 0;
}

// Schedule cars in road.
// If car can through cross then car into schedule wait -> car.schedule_status = 1
// If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
// If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 0
// If car blocked by termination state car then car move to the back of the previous car -> car.schedule_status = 0
void overall_schedule::schedule_cars_running_in_road() {
    // schedule all cars' state which running in road
    this->cars_running_wait_state_n = 0;
    for (list<road>::iterator iter = this->roads_connect_cross.begin(); iter != this->roads_connect_cross.end(); iter ++) {
        // schedule cars in road
        this->cars_running_wait_state_n += iter->schedule_cars_running_in_road();
    }
    // update roads' state
    for (map<int, cross>::iterator iter = crosses.begin(); iter != crosses.end(); iter ++) {
        // update road state in cross
        iter->second.update_road_state_in_cross();
    }
}

// schedule cars in one time unit
// if deadblock return false
bool overall_schedule::schedule_cars_one_time_unit() {
    // Schedule cars in road.
    // If car can through cross then car into schedule wait -> car.schedule_status = 1
    // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
    // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
    // If car blocked by end state car then car move to the back of the previous car
    schedule_cars_running_in_road();
    cout << "cars_running_wait_state_n = " << this->cars_running_wait_state_n << endl;
    /*
    // Cycling the relevant roads at each cross until all car are end state
    while (car_wait_in_cross_n > 0) {
        for (all crosses) {
            // todo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            schedule_cars_in_cross()
        }
    }
    // Schedule cars which wait start
    schedule_cars_wait_run();
    */
    return true;
}

int overall_schedule::schedule_cars() {
    int T = 0;
    // initial state
    this->initial_cars_state_in_T0();
    // If all cars is arrive then break
    while (this->cars_wait_schedule_start_time_n > 0 || this->cars_wait_run_n > 0 || this->cars_running_n > 0) {
        if (!this->schedule_cars_one_time_unit())
            return -1;
        T ++;
        break;
    }
    return T;
}