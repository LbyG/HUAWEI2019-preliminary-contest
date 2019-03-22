#ifndef OVERALL_SCHEDULE
#define OVERALL_SCHEDULE

#include <queue>
#include <map>
#include <list>

#include "car.h"
#include "road.h"
#include "cross.h"
#include "cmp_car_plan_time.h"
#include "cmp_car_id.h"

using namespace std;

class overall_schedule {
private:
    int cars_wait_plan_time_n; // number of cars which T < car.plan_time
    priority_queue<car, vector<car>, cmp_car_plan_time> cars_wait_plan_time_list; // cars which T < car.plan_time, priority depend on planTime
    int cars_wait_run_n; // the number of cars wait to start run
    priority_queue<car, vector<car>, cmp_car_id> cars_wait_run_list; // cars wait to start run, priority depend on id
    int cars_running_n; // the number of cars is running
    int car_wait_in_cross_n; // the number of cars wait to through cross or previous car is wait to through cross 
    
    map<int, car> cars; // cars[car.id] = car
    map<int, road> roads; // roads[road.id] = road
    map<int, cross> crosses; // crosses[cross.id] = cross
public:
    // return cars number
    int get_cars_n();
    // function to clear priority queue
    void clear_priority_queue(priority_queue<car, vector<car>, cmp_car_plan_time> &pq);
    
    //load cars, roads and crosses info from car_path, road_path and cross_path file
    void load_cars_road_cross(string car_path, string road_path, string cross_path);
    // load cars' path from answer_path
    void load_answer(string answer_path);
    // Initial all running cars schedule status, car.schedule_status = 0
    void initial_cars_schedule_status(); 
    // Schedule cars in road.
    // If car can through cross then car into schedule wait -> car.schedule_status = 1
    // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
    // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
    // If car blocked by end state car then car move to the back of the previous car
    void schedule_cars_in_road();
    int schedule_cars(); // return all cars arrive to_cross_id need how much times
};

#endif