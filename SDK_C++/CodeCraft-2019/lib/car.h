#ifndef CAR_H
#define CAR_H

class car {
    public:
    int car_id; // car id
    int schedule_status; // schedule_status == 0 schedule wait; schedule_status == 1 cross wait; schedule_status == 2 end state;
    car();
};

#endif