#include "road.h"

#include <vector>
#include <iostream>

using namespace std;

road::road() {
    // TODO
    this->id = 0;
}

road::road(string road_info) {
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    int len = road_info.size();
    vector<int> info_val;
    info_val.clear();
    int previous_char_pos = -1;
    int val = 0;
    for (int i = 0; i < len; ++i) {
        if (road_info[i] < '0' || road_info[i] > '9') {
            if (i > (previous_char_pos + 1)) {
                info_val.push_back(val);
                val = 0;
            }
            previous_char_pos = i;
        } else {
            val = val * 10 + (int(road_info[i]) - int('0'));
        }
    }
    this->id = info_val[0];
    this->length = info_val[1];
    this->speed = info_val[2];
    this->channel = info_val[3];
    this->from = info_val[4];
    this->to = info_val[5];
    this->is_duplex = info_val[6];
    
    cars_in_road.clear();
    for (int i = 0; i < channel; i ++) {
        cars_in_road.push_back(list<car>());
    }
}

// return road id
int road::get_id() const {
    return this->id;
}

// return road from cross id
int road::get_from() const {
    return this->from;
}

// return road to cross id
int road::get_to() const {
    return this->to;
}

// return is_duplex
int road::get_is_duplex() const {
    return this->is_duplex;
}

// if through road can arrive cross_id, if cross_id == to || (cross_id == from && isDuplex == 1) return true else return false
bool road::ifArriveCross(int cross_id) {
    if (cross_id == this->to)
        return true;
    else if (this->is_duplex == 1 && cross_id == this->from)
        return true;
    else
        return false;
}
