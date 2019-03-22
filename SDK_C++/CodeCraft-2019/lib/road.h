#ifndef ROAD_H
#define ROAD_H

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
    int isDuplex;
    // situation of cars in road
    // cars_in_road[channel_id] = list(car1 -> car2 -> car3 -> ...)
    // channel_id = [0, channel-1]
    // list<car> = car1->car2->car3->car4, ... the distance from the arrive_cross_id is from near to far
    vector<list<car>> cars_in_road;
public:
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    road(string road_info);
    // if through road can arrive cross_id, if cross_id == to || (cross_id == from && isDuplex == 1) return true else return false
    bool ifArriveCross(int cross_id);
};

#endif