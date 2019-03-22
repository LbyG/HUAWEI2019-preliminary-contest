#ifndef ROAD_H
#define ROAD_H

#include "car.h"

#include <vector>
#include <list>

using namespace std;

class road {
private:
    int id; // road id
    int length; // road length
    int speed; // road max speed limit
    int channel; // road channel number
    int from; // road from cross id
    int to; // road to cross id
    // isDuplex == 1, road can from from_cross_id to to_cross_id
    // isDuplex == 1, road can from from_cross_id to to_cross_id and from to_cross_id to from_cross_id
    int is_duplex;
    // situation of cars in road
    // cars_in_road[channel_id] = list(car1 -> car2 -> car3 -> ...)
    // channel_id = [0, channel-1]
    // list<car> = car1->car2->car3->car4, ... the distance from the arrive_cross_id is from near to far
    vector<list<car>> cars_in_road;
public:
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    road();
    road(string road_info);
    int get_id() const; // return road id
    int get_from() const; // return road from cross id
    int get_to() const; // return road to cross id
    int get_is_duplex() const; // return is_duplex
    // if through road can arrive cross_id, if cross_id == to || (cross_id == from && isDuplex == 1) return true else return false
    bool ifArriveCross(int cross_id);
};

#endif