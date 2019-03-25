#ifndef OVERALL_SCHEDULE
#define OVERALL_SCHEDULE

#include <queue>
#include <map>
#include <list>

#include "car.h"
#include "road.h"
#include "cross.h"
#include "cmp_car_schedule_start_time.h"
#include "cmp_car_id.h"

using namespace std;

class overall_schedule {
private:
    int T;
    int all_cars_running_time;
    // N = cars_wait_schedule_start_time_n + cars_wait_run_n + cars_running_n + cars_arrive_destination_n
    // number of cars which T < car.schedule_start_time
    int cars_wait_schedule_start_time_n; 
    // cars which T < car.schedule_start_time, priority depend on schedule_start_time
    priority_queue<car, vector<car>, cmp_car_schedule_start_time> cars_wait_schedule_start_time_list; 
    // the number of cars which T >= car.schedule_start_time but wait to running in road
    int cars_wait_run_n; 
    // cars which T >= car.schedule_start_time but wait to running in road, priority depend on id
    vector<car> cars_wait_run_list;
    // the number of cars which is running in road
    int cars_running_n; 
    // the number of cars which is arrive destination
    int cars_arrive_destination_n;
    
    // cars_running_n = cars_wait_state_n + cars_termination_state_n
    // the number of cars wait to through cross or previous car is wait to through cross
    int cars_running_wait_state_n; 
    // the number of cars was run in this time unit
    int cars_running_termination_state_n;
    
    // cars[car.id] = car
    map<int, car> cars; 
    // roads[road.id] = road
    map<int, road> roads; 
    // crosses[cross.id] = cross
    map<int, cross> crosses;
    // vector of roads connect to cross
    list<road> roads_connect_cross;
public:
    // return cars number
    int get_cars_n();
    
    //load cars, roads and crosses info from car_path, road_path and cross_path file
    void load_cars_roads_crosses(string car_path, string road_path, string cross_path);
    // load cars' path from answer_path
    void load_answer(string answer_path);
    
    // initial all data in T = 0
    void initial_cars_state_in_T0();
    // Schedule cars in road.
    // If car can through cross then car into schedule wait -> car.schedule_status = 1
    // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
    // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
    // If car blocked by end state car then car move to the back of the previous car
    void schedule_cars_running_in_road();
    // Schedule cars which arrive schedule time or wait start
    // cars_wait_schedule_start_time_list -> cars_wait_run_list
    // cars_wait_run_list -> cars_in_road
    void schedule_cars_wait_run();
    
    // schedule cars in one time unit
    // if deadblock return false
    bool schedule_cars_one_time_unit();
    // return all cars arrive to_cross_id need how much times
    int schedule_cars(); 
    
    // output car schedule status
    void output_schedule_status();
};

#endif