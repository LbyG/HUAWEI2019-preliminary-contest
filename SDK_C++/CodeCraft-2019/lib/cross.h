#ifndef CROSS_H
#define CROSS_H

class cross {
    public:
    int cross_id; // cross id
    // turn_direct[road_id1][road_id2] = 0 road_id1 -> road_id2 Straight;
    // turn_direct[road_id1][road_id2] = 1 road_id1 -> road_id2 Turn left;
    // turn_direct[road_id1][road_id2] = 2 road_id1 -> road_id2 Turn right;
    vector<int, vector<int, int>> turn_direct; 
    cross();
};

#endif