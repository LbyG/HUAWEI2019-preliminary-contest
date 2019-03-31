#ifndef ROAD_H
#define ROAD_H

#include "car.h"
#include "cmp_car_dis_to_cross_and_channel_id.h"

#include <vector>
#include <list>
#include <queue>

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
    
    // channel 0 to into_channel_id-1 can't into, initial value is 0 and into_channel_id < channel
    int into_channel_id;
    // size = 3, wait_into_road_direction_count[0] is straight, wait_into_road_direction_count[1] is left, wait_into_road_direction_count[2] is right
    vector<int> wait_into_road_direction_count;
    // dis_to_cross from small to large
    // if dis_to_cross equal, channel_id from small to large
    priority_queue<car, vector<car>, cmp_car_dis_to_cross_and_channel_id> wait_car_forefront_of_each_channel;
    
    // situation of cars in road
    // cars_in_road[channel_id] = list(car1 -> car2 -> car3 -> ...)
    // channel_id = [0, channel-1]
    // list<car> = car1->car2->car3->car4, ... the distance from the arrive_cross_id is from near to far
    vector<list<car>> cars_in_road;
public:
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    road(string road_info);
    int get_id() const; // return road id
    int get_from() const; // return road from cross id
    int get_to() const; // return road to cross id
    int get_is_duplex() const; // return is_duplex
    // swap from and to
    void swap_from_to();
    
    // set.into_channel_id = into_channel_id
    void set_into_channel_id(int into_channel_id);
    // road.wait_road_direction_count = [0, 0, 0]
    void init_wait_into_road_direction_count();
    // road.wait_car_forefront_of_each_channel.clear();
    void clear_wait_car_forefront_of_each_channel();
    
    // if no car need through cross in this road
    bool if_no_car_through_cross();
    // get car which have priority through cross, wait_car_forefront_of_each_channel
    car get_car_priority_through_cross();
    // have car through cross
    void have_car_through_cross(int channel_id);
    
    // wait_into_road_direction_count[car_direct] += 1
    void add_wait_into_road_direction_count(int car_direct);
    // wait_into_road_direction_count[car_direct] -= 1
    void sub_wait_into_road_direction_count(int car_direct);
    // check whether car_turn_direct can enter this road
    bool check_direct_priority(int car_turn_direct);
    
    // init cars' schedule state to wait state which in channel
    // return number of cars in channel
    int init_cars_schedule_status_in_channel(int channel_id);
    // Schedule cars in road.
    // If car can through cross then car into schedule wait -> car.schedule_status = 1
    // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
    // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 0
    // If car blocked by termination state car then car move to the back of the previous car -> car.schedule_status = 0
    // return number of from wait state to termination status
    int schedule_cars_running_in_channel(int channel_id, int flag, int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T);
    int schedule_cars_running_in_road(int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T);
    // forefront car because some reason it need remain in cross
    int forefront_car_remain_in_cross(int channel_id, int flag, int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T);
    
    // check the road all channel whether be fill up
    // be fill up -> true else -> false
    bool whether_be_fill_up();
    // if car speed_in_road <= car.dis_to_cross in previous road, then dis_move_in_road = 0 -> car don't enter this road, car.dis_to_cross = 0 and return -1 
    // else if car into road don't be block or block by a car which is termination status, car enter road, return 1
    // else car can't enter road, need wait previous car to be termination state return 0
    int car_into_road(car into_car, int flag);
    
    // output road status
    void output_status(int T);
};

bool operator<(const road &a, const road &b);

#endif