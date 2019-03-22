#ifndef CROSS_H
#define CROSS_H

class cross {
private:
    int id; // cross id
    // turn_direct[road_id1][road_id2] = 0 road_id1 -> road_id2 Straight;
    // turn_direct[road_id1][road_id2] = 1 road_id1 -> road_id2 Turn left;
    // turn_direct[road_id1][road_id2] = 2 road_id1 -> road_id2 Turn right;
    map<int, map<int, int>> turn_direct;
    // situation of road can arrive this cross, road_in_cross[i].id < road_in_cross[i+1].id
    vector<road> road_into_cross;
    // road_departure_cross[road_id] -> road which id == road_id and from this cross to other cross
    map<int, road*> road_departure_cross;
public:
    // convert cross_info = (id,roadId1,roadId2,roadId3,roadId4) - > 
    // turn_direct[roadId1][roadId2] = 1     turn left 
    // turn_direct[roadId1][roadId3] = 0     straight
    // turn_direct[roadId1][roadId3] = 2     turn right
    // ...
    cross(string cross_info); 
    int get_cross_id(); // return cross id
    // query in this cross from from_road_id to to_road_id is straight? left? or right?
    int get_turn_direct(int from_road_id, int to_road_id);
};

#endif