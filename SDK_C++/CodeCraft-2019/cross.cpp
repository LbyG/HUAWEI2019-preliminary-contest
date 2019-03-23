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
void cross::update_road_state_in_cross() {
    for (vector<road*>::iterator iter = roads_into_cross.begin(); iter != roads_into_cross.end(); iter ++) {
        int now_road_id = (*iter)->get_id();
        if ((*iter)->if_no_car_through_cross())
            continue;
        int next_road_id = (*iter)->get_car_priority_through_cross().get_next_road_in_path();
        if (next_road_id == -1)
            continue;
        int car_direct = this->turn_direct[now_road_id][next_road_id];
        this->roads_departure_cross[next_road_id]->add_wait_into_road_direction_count(car_direct);
        
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
