#include "cross.h"

#include "road.h"

#include <vector>
#include <iostream>

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
    int len = cross_info.size();
    vector<int> info_val;
    info_val.clear();
    int previous_char_pos = -1;
    int val = 0;
    int flag = 1;
    for (int i = 0; i < len; ++i) {
        if (cross_info[i] < '0' || cross_info[i] > '9') {
            if (i > (previous_char_pos + 1)) {
                info_val.push_back(flag * val);
                val = 0;
                flag = 1;
            }
            if (cross_info[i] == '-')
                flag = -1;
            previous_char_pos = i;
        } else {
            val = val * 10 + (int(cross_info[i]) - int('0'));
        }
    }
    this->id = info_val[0];
    this->road_into_cross.clear();
    this->road_departure_cross.clear();
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
    this->road_into_cross.push_back(road_pointer);
}

// add road* to road_departure_cross which road->from == cross_id
void cross::add_road_departure_cross(int road_id, road* road_pointer) {
    this->road_departure_cross[road_id] = road_pointer;
}
