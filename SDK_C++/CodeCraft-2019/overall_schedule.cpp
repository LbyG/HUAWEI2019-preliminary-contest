#include "overall_schedule.h"

#include "util.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

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
        this->crosses[from_to_road->get_to()].add_road_into_cross(from_to_road);
        this->crosses[from_to_road->get_from()].add_road_departure_cross(iter->first, from_to_road);
        // if isDuplex == 1m road from to_id to from_id connect to crosses
        if (iter->second.get_is_duplex() == 1) {
            this->roads_connect_cross.push_back(road(iter->second));
            this->roads_connect_cross.back().swap_from_to();
            road* to_from_road = &this->roads_connect_cross.back();
            this->crosses[to_from_road->get_to()].add_road_into_cross(to_from_road);
            this->crosses[to_from_road->get_from()].add_road_departure_cross(iter->first, to_from_road);
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
    this->T = 0;
    this->all_cars_running_time = 0;
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
    this->cars_wait_run_list.clear(); 
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

// Schedule cars which arrive schedule time or wait start
// cars_wait_schedule_start_time_list -> cars_wait_run_list
// cars_wait_run_list -> cars_in_road
void overall_schedule::schedule_cars_wait_run() {
    while (!this->cars_wait_schedule_start_time_list.empty() && this->cars_wait_schedule_start_time_list.top().get_schedule_start_time() == this->T) {
        this->cars_wait_run_list.push_back(this->cars_wait_schedule_start_time_list.top());
        this->cars_wait_run_n ++;
        this->cars_wait_schedule_start_time_list.pop();
        this->cars_wait_schedule_start_time_n --;
    }
    sort(this->cars_wait_run_list.begin(), this->cars_wait_run_list.end());
    vector<car> car_still_wait_run;
    car_still_wait_run.clear();
    for (vector<car>::iterator iter = this->cars_wait_run_list.begin(); iter != this->cars_wait_run_list.end(); iter ++) {
        int flag = this->crosses[iter->get_from()].car_to_next_road(*iter);
        if (flag == 1) {
            this->cars_wait_run_n --;
            this->cars_running_n ++;
        } else if (flag == -2) {
            
            car_still_wait_run.push_back(*iter);
        } else {
            cout << "overall_schedule::schedule_cars_wait_run  error !!!!!!!!!!!!!!!!!!!" << endl;
        }
    }
    this->cars_wait_run_list = car_still_wait_run;
}

// schedule cars in one time unit
// if deadblock return false
bool overall_schedule::schedule_cars_one_time_unit() {
    // Schedule cars in road.
    // If car can through cross then car into schedule wait -> car.schedule_status = 1
    // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
    // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
    // If car blocked by end state car then car move to the back of the previous car
    this->schedule_cars_running_in_road();
    
//    this->output_schedule_status();
    
    // Cycling the relevant roads at each cross until all car are end state
    while (this->cars_running_wait_state_n > 0) {
        int wait_to_termination_n = 0;
        for (map<int, cross>::iterator iter = crosses.begin(); iter != crosses.end(); iter ++) {
            wait_to_termination_n += iter->second.schedule_cars_in_cross(this->cars_running_n, this->cars_arrive_destination_n, this->all_cars_running_time, this->T);
        }
        if (wait_to_termination_n == 0) {
            cout << "cars_running_wait_state_n = " << this->cars_running_wait_state_n << " wait_to_termination_n = " << wait_to_termination_n << endl; 
            this->output_schedule_status();
            return false;
        }
        this->cars_running_wait_state_n -= wait_to_termination_n;
    }
    
    // Schedule cars which arrive schedule time or wait start
    // cars_wait_schedule_start_time_list -> cars_wait_run_list
    // cars_wait_run_list -> cars_in_road
    this->schedule_cars_wait_run();
    
//    this->output_schedule_status();
    return true;
}

int overall_schedule::schedule_cars() {
    // initial state
    this->initial_cars_state_in_T0();
    // If all cars is arrive then break
    while (this->cars_wait_schedule_start_time_n > 0 || this->cars_wait_run_n > 0 || this->cars_running_n > 0) {
        if (!this->schedule_cars_one_time_unit()) {
            cout << "deadblock!!!!!!!!!!!!!!!" << endl;
            return -1;
        }
        this->T ++;
    }
    cout << "all cars running time = " << this->all_cars_running_time << endl;
    return this->T;
}

// output car schedule status
void overall_schedule::output_schedule_status() {
    cout << "========show schedule status=========" << endl;
    cout << "T = " << this->T << endl;
    cout << "cars_wait_schedule_start_time_n = " << this->cars_wait_schedule_start_time_n << endl;
    cout << "cars_wait_run_n = " << this->cars_wait_run_n << endl;
    cout << "car is list which wait run: ";
    for (vector<car>::iterator iter = this->cars_wait_run_list.begin(); iter != this->cars_wait_run_list.end(); iter ++) {
        cout << iter->get_id() << " ";
    }
    cout << endl;
    cout << "cars_running_n = " << this->cars_running_n << endl;
    cout << "cars_arrive_destination_n = " << this->cars_arrive_destination_n << endl;
    cout << "cars_running_wait_state_n = " << this->cars_running_wait_state_n << endl;
    cout << "cars_running_termination_state_n = " << this->cars_running_termination_state_n << endl;
    cout << "car running in the road:" << endl;
    for (list<road>::iterator iter = roads_connect_cross.begin(); iter != roads_connect_cross.end(); iter ++) {
 //       if (!iter->if_no_car_through_cross())
            iter->output_status();
    }
    cout << "=====================================" << endl;
}