#ifndef CAR_H
#define CAR_H

#include <string>

using namespace std;

class car {
private:
    int id; // car id
    int from; // start from which cross_id
    int to; // arrive to which cross_id;
    int speed; // car max speed limit
    int plan_time; // car plan start time
    int schedule_status; // schedule_status == 0 schedule wait; schedule_status == 1 cross wait; schedule_status == 2 end state;
public:
    car();
    car(string car_info); // car_info = (id,from,to,speed,planTime)
    int get_id() const; // return car id
    int get_from() const; // reutrn car start from cross_id;
    int get_to() const; // return car arrive to cross_id;
    int get_speed() const; // return car speed
    int get_plan_time() const; // return car plan start time
    void set_schedule_status(int schedule_status); // set this -> schedule_status = schedule_status
    int get_schedule_status() const; // return car schedule status
};

#endif