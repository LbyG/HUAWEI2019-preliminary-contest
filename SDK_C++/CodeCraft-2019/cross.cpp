#include "cross.h"

#include "road.h"
#include "util.h"

#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

cross::cross() {
    // TODO
    this->id = 0;
}

// convert cross_info = (id,roadId1,roadId2,roadId3,roadId4) - > 
// turn_direct[roadId1][roadId2] = 1     turn left 
// turn_direct[roadId1][roadId3] = 0     straight
// turn_direct[roadId1][roadId3] = 2     turn right
// ...
cross::cross(string cross_info) {
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    vector<int> info_val = parse_string_to_int_vector(cross_info);
    
    this->id = info_val[0];
    this->roads_into_cross.clear();
    this->roads_departure_cross.clear();
    // generate turn direct
    this->turn_direct.clear();
    this->road_id_in_cross.clear();
    for (int i = 0; i < 4; i ++) {
        info_val[i] = info_val[i + 1];
        if (info_val[i] != -1)
            this->road_id_in_cross.push_back(info_val[i]);
    }
    for (int i = 0; i < 4; i ++) {
        turn_direct[info_val[i]][info_val[(i + 1) % 4]] = 1;
        turn_direct[info_val[i]][info_val[(i + 2) % 4]] = 0;
        turn_direct[info_val[i]][info_val[(i + 3) % 4]] = 2;
    }
}

// return cross id
int cross::get_id() const {
    return this->id;
}

// query in this cross from from_road_id to to_road_id is 0->straight? 1->left? or 2->right?
int cross::get_turn_direct(int from_road_id, int to_road_id) {
    return this->turn_direct[from_road_id][to_road_id];
}

// add road* to road_into_cross which road->to = cross_id
void cross::add_road_into_cross(road* road_pointer) {
    this->roads_into_cross.push_back(road_pointer);
}

// add road* to road_departure_cross which road->from == cross_id
void cross::add_road_departure_cross(int road_id, road* road_pointer) {
    this->roads_departure_cross[road_id] = road_pointer;
}

// update road state in cross
// if for a road have 1 straight, 1 left then road.wait_into_road_direction_count = [1, 1, 0]
void cross::update_road_state_in_cross(road* road_pointer) {
    int now_road_id = road_pointer->get_id();
    if (road_pointer->if_no_car_through_cross())
        return;
    int next_road_id = road_pointer->get_car_priority_through_cross().get_next_road_in_path();
    if (next_road_id == -1)
        // car will arrive destination
        return;
    int car_direct = this->turn_direct[now_road_id][next_road_id];
    this->roads_departure_cross[next_road_id]->add_wait_into_road_direction_count(car_direct);
}

// update road state in cross
// if for a road have 1 straight, 1 left then road.wait_into_road_direction_count = [1, 1, 0]
void cross::update_road_state_in_cross() {
    for (vector<road*>::iterator iter = roads_into_cross.begin(); iter != roads_into_cross.end(); iter ++) {
        this->update_road_state_in_cross(*iter);
    }
}

// sort roads_into_cross by road_id;
void cross::roads_into_cross_sort_by_id() {
    //sort(roads_into_cross.begin(),roads_into_cross.end());
    for (vector<road*>::iterator iter = roads_into_cross.begin(); iter != roads_into_cross.end(); iter ++) {
        cout << (*iter)->get_id() << " ";
        //cout << (*iter) << " ";
    }
    cout << endl;
    for (vector<int>::iterator iter = road_id_in_cross.begin(); iter != road_id_in_cross.end(); iter ++) {
        cout << (*iter) << " ";
        //cout << (*iter) << " ";
    }
    cout << endl;
}

// car to next road, maybe return -2, -1, 0, 1
// if road is be termination status cars fill up return -2
// if car speed_in_road <= car.dis_to_cross in previous road, then dis_move_in_road = 0 -> car don't enter this road, car.dis_to_cross = 0 and return -1 
// else if car into road don't be block or block by a car which is termination status, car enter road, return 1
// else car can't enter road, need wait previous car to be termination state return 0
int cross::car_to_next_road(car car_through_cross) {
    road* next_road = this->roads_departure_cross[car_through_cross.get_next_road_in_path()];
    if (next_road->whether_be_fill_up())
        return -2;
    return next_road->car_into_road(car_through_cross);
}

// schedule cars can through cross
// return number of cars from wait status to termination status in this schedule
int cross::schedule_cars_in_cross(int &cars_running_n, int &cars_arrive_destination_n) {
    int wait_to_termination_n = 0;
    for (vector<road*>::iterator iter = this->roads_into_cross.begin(); iter != this->roads_into_cross.end(); iter ++) {
        while (!(*iter)->if_no_car_through_cross()) {
            car car_through_cross = (*iter)->get_car_priority_through_cross();
            if (car_through_cross.get_next_road_in_path() == -1) {
                // car arrive destination
                wait_to_termination_n ++;
                cars_running_n --;
                cars_arrive_destination_n ++;
                int channel_id = car_through_cross.get_channel_id();
                (*iter)->have_car_through_cross(channel_id);
                wait_to_termination_n += (*iter)->schedule_cars_running_in_channel(channel_id);
                continue;
            }
            road* next_road = this->roads_departure_cross[car_through_cross.get_next_road_in_path()];
            int car_turn_direct = this->turn_direct[((*iter)->get_id())][next_road->get_id()];
            if (next_road->check_direct_priority(car_turn_direct)) {
                // car direct have priority enter to road
                int channel_id = car_through_cross.get_channel_id();
                if (next_road->whether_be_fill_up()) {
                    // road is be fill up by cars which is termination status
                    wait_to_termination_n += (*iter)->forefront_car_remain_in_cross(channel_id);
                    next_road->sub_wait_into_road_direction_count(car_turn_direct);
                    continue;
                } else {
                    // road don't be fill up
                    int flag = next_road->car_into_road(car_through_cross);
                    if (flag == -1) {
                        // car speed in next raod < dis_to_cross, so car be remain in cross
                        wait_to_termination_n += (*iter)->forefront_car_remain_in_cross(channel_id);
                        next_road->sub_wait_into_road_direction_count(car_turn_direct);
                        continue;
                    } else if (flag == 0) {
                        // car block by car which is wait state
                        break;
                    } else if (flag == 1) {
                        // car enter next road
                        wait_to_termination_n ++;
                        (*iter)->have_car_through_cross(channel_id);
                        wait_to_termination_n += (*iter)->schedule_cars_running_in_channel(channel_id);
                        // update direct priority
                        next_road->sub_wait_into_road_direction_count(car_turn_direct);
                        this->update_road_state_in_cross(*iter);
                    }
                }
            } else {
                // need wait other direct to through
                break;
            }
            
        }
    }
    return wait_to_termination_n;
}
