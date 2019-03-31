import logging
import sys

import math
import heapq
import random
import numpy as np

logging.basicConfig(level=logging.DEBUG,
                    filename='../logs/CodeCraft-2019.log',
                    format='[%(asctime)s] %(levelname)s [%(funcName)s: %(filename)s, %(lineno)d] %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    filemode='a')

# set random seed
random.seed(2050)
np.random.seed(2050)

# p_d = path_random_choose_index
# ((1/dis_i)^p_d)/((1/dis_1)^p_d + (1/dis_2)^p_d + (1/dis_3)^p_d) probability to choose road_i
path_random_choose_index = 0
#max_running_car_num = 500
# cars max running time
CAR_RUNNING_TIME_LENGTH = 2500
# road["capacity"] = road["channel"] * max(1, road["length"] - road["speed"]) * CAPACITY_WEIGHT // 1
CAPACITY_WEIGHT = 0.5
# Divide vehicles into epoch-making batches for departure arrangements
epoch = 1
# True: ratio = capacity / capacity_sum, False: ratio = 1 / capacity_sum
ifRemainCapacityRatio = True

def read_car_file(car_path):
# deal car info
# cars = [{id:1, frome:1, to:1, speed:1, planTime:1} , ...]
    cars = []
    car_attr = []
    with open(car_path, 'r') as r_file:
        car_info = r_file.readlines()
        for info in car_info:
            info = info.strip()
            # 特殊处理注释
            if info[0] == "#":
                car_attr = [attr for attr in info[2:-1].split(",")]
                continue
            info = [eval(val) for val in info[1:-1].split(", ")]
            if len(car_attr) != len(info):
                logging.info('car info missing attribute')
                exit(1)
            car = {}
            for i in range(len(car_attr)):
                car[car_attr[i]] = info[i]
            car["isOnTheRoad"] = -1
            cars.append(car)
    #cars.sort(key = lambda x:x["planTime"])
    return cars, car_attr

def read_cross_file(cross_path):
# deal cross info
# cross = {id1:[roadId1, roadId2, roadId3, roadId4], id2:{} ...}
    crosses = {}
    crossN = 0
    with open(cross_path, 'r') as r_file:
        cross_info = r_file.readlines()
        for info in cross_info:
            info = info.strip()
            # 特殊处理注释
            if info[0] == "#":
                continue
            info = [eval(val) for val in info[1:-1].split(", ")]
            crosses[info[0]] = info[1:]
            crossN += 1
    return crosses, crossN

def read_road_file(road_path, crosses, cars):
# deal road info
# roads = {id:{id, length, speed, channel, from, to, isDuplex}, ...}
# cross_with_road connect cross info and road info
# cross_with_road[road[to]].append(road)
# if isDuplex == 1 cross_with_road[road[from]].append(road)
    roads = {}
    road_attr = []
    cross_with_to_road = {}
    cross_with_from_road = {}
    for cross_id in crosses: 
        cross_with_to_road[cross_id] = list()
        cross_with_from_road[cross_id] = list()
    with open(road_path, 'r') as r_file:
        road_info = r_file.readlines()
        for info in road_info:
            info = info.strip()
            # 特殊处理注释
            if info[0] == "#":
                road_attr = [attr for attr in info[2:-1].split(",")]
                continue
            info = [eval(val) for val in info[1:-1].split(", ")]
            if len(road_attr) != len(info):
                logging.info('road info missing attribute')
                exit(1)
            road = {}
            for i in range(len(road_attr)):
                road[road_attr[i]] = info[i]
            capacity = road["channel"] * max(1, road["length"] - road["speed"]) * CAPACITY_WEIGHT // 1
            road["capacity"] = capacity
            roads[road["id"]] = road
            road_reverse = road.copy()
            road_reverse["from"] = road["to"]
            road_reverse["to"] = road["from"]
            cross_with_to_road[road["to"]].append(road_reverse.copy())
            cross_with_from_road[road["from"]].append(road.copy())
            if road["isDuplex"] == 1:
                cross_with_to_road[road_reverse["to"]].append(road.copy())
                cross_with_from_road[road_reverse["from"]].append(road_reverse.copy())
    #(id,from,to,speed,planTime)
    """
    if cars[0]["from"] == 18 and cars[0]["to"] == 50 and cars[0]["speed"] == 8 and cars[0]["planTime"] == 3:
        print("11111111111111111111111")
        #deadblock_cross_id = [37, 35, 12, 29, 21, 28, 11, 43, 21, 36, 53]
        deadblock_cross_id = []
        for cross_id in deadblock_cross_id:
            for road_id in crosses[cross_id]:
                if road_id != -1:
                    roads[road_id]["capacity"] = roads[road_id]["capacity"] * 0.8 // 1
    else:
        #print("22222222222222222222222")
        #deadblock_cross_id = [26, 11, 11, 11, 11, 13, 11, 13, 6, 51, 13, 29, 13, 11, 36]
        #for cross_id in deadblock_cross_id:
        #    for road_id in crosses[cross_id]:
        #        if road_id != -1:
        #            roads[road_id]["capacity"] = roads[road_id]["capacity"] * 0.8 // 1
        for road_id in roads:
            roads[road_id]["capacity"] = roads[road_id]["capacity"] * 0.8 // 1
   """
    return cross_with_to_road, cross_with_from_road, roads, road_attr

def find_shortest_path_state_update(update_cross_id, cross_with_road, cross_arrive_state, car_speed):
# Dijkstra algorithm to update arrive cross state
    for road in cross_with_road[update_cross_id]:
        road_through_time = math.ceil(road["length"] / min(car_speed, road["speed"]))
        road_to_id = road["to"]
        if cross_arrive_state[road_to_id]["time"] > cross_arrive_state[update_cross_id]["time"] + road_through_time:
            cross_arrive_state[road_to_id]["time"] = cross_arrive_state[update_cross_id]["time"] + road_through_time
            cross_arrive_state[road_to_id]["crossFromId"] = update_cross_id
            cross_arrive_state[road_to_id]["roadFromId"] = road["id"]

def find_shortest_path(car, cross_with_road):
# Dijkstra algorithm to find from car["from"] to car["to"] shortest path
    MAX_TIME = 1e8
    car_from_id = car["from"]
    car_to_id = car["to"]
    car_speed = car["speed"]
    cross_arrive_state = {}
    for cross_id in cross_with_road:
        cross_arrive_state[cross_id] = {"time":MAX_TIME, "crossFromId":-1, "roadFromId":-1, "isArrive":0}
    cross_arrive_state[car_from_id] = {"time":0, "crossFromId":car_from_id, "roadFromId":0, "isArrive":1}
    find_shortest_path_state_update(car_from_id, cross_with_road, cross_arrive_state, car_speed)
    while cross_arrive_state[car_to_id]["isArrive"] == 0:
        update_cross_id, update_cross_time = -1, MAX_TIME
        for cross_id in cross_arrive_state:
            state = cross_arrive_state[cross_id]
            if state["isArrive"] == 0:
                if state["time"] < update_cross_time:
                    update_cross_time = state["time"]
                    update_cross_id = cross_id
        cross_arrive_state[update_cross_id]["isArrive"] = 1
        find_shortest_path_state_update(update_cross_id, cross_with_road, cross_arrive_state, car_speed)
    run_time = cross_arrive_state[car_to_id]["time"]
    path = []
    cross_id = car_to_id
    while cross_id != car_from_id:
        path.append(cross_arrive_state[cross_id]["roadFromId"])
        cross_id = cross_arrive_state[cross_id]["crossFromId"]
    path.reverse()
    return run_time, path

def get_near_cross_sequence(car, cross_with_to_road):
# Dijkstra algorithm to find the cross sequence arrive_car_list which is from near to far dis for car_to_cross
    MAX_NUM = 1e8
    arrive_cross_N = 0
    arrive_cross_list = []
    car_from_id = car["from"]
    car_to_id = car["to"]
    car_speed = car["speed"]
    cross_arrive_state = {}
    for cross_id in cross_with_to_road:
        cross_arrive_state[cross_id] = {"time":MAX_NUM, "crossFromId":-1, "roadFromId":-1, "arriveOrder":MAX_NUM}
    cross_arrive_state[car_to_id] = {"time":0, "crossFromId":car_from_id, "roadFromId":0, "arriveOrder":arrive_cross_N}
    arrive_cross_N += 1
    arrive_cross_list.append(car_to_id)
    find_shortest_path_state_update(car_to_id, cross_with_to_road, cross_arrive_state, car_speed)
    while True:
        update_cross_id, update_cross_time = -1, MAX_NUM
        for cross_id in cross_arrive_state:
            state = cross_arrive_state[cross_id]
            if state["arriveOrder"] == MAX_NUM and cross_id != car_from_id:
                if state["time"] < update_cross_time:
                    update_cross_time = state["time"]
                    update_cross_id = cross_id
        if update_cross_id == -1:
            break
        cross_arrive_state[update_cross_id]["arriveOrder"] = arrive_cross_N
        arrive_cross_N += 1
        arrive_cross_list.append(update_cross_id)
        find_shortest_path_state_update(update_cross_id, cross_with_to_road, cross_arrive_state, car_speed)
    arrive_cross_list.append(car_from_id)
    return arrive_cross_list, cross_arrive_state

def calc_expect_run_time_and_prob(arrive_cross_list, cross_arrive_state, cross_with_from_road, car_speed):
# calc expect run time and prob
# cross_with_optional_road = {cross_id:[{crossToId, roadToId, expectTime, prob, throughTime}]}
    cross_with_optional_road = {}
    cross_arrive_state[arrive_cross_list[0]]["time"] = 0
    for cross_id in arrive_cross_list[1:]:
        cross_arrive_state[cross_id]["time"] = 0
        optional_road_list = list()
        sum_expect_time = 0
        for road in cross_with_from_road[cross_id]:
            if cross_arrive_state[road["to"]]["arriveOrder"] < cross_arrive_state[cross_id]["arriveOrder"]:
                road_through_time = math.ceil(road["length"] / min(car_speed, road["speed"]))
                expect_time = road_through_time + cross_arrive_state[road["to"]]["time"]
                sum_expect_time += expect_time
                optional_road_list.append({"crossToId":road["to"], "roadToId":road["id"], "expectTime":expect_time, "prob":0, "throughTime":road_through_time})
        prob_sum = 0
        for optional_road in optional_road_list:
            optional_road["prob"] = 1 / math.pow(optional_road["expectTime"], path_random_choose_index)
            prob_sum += optional_road["prob"]
        for optional_road in optional_road_list:
            optional_road["prob"] /= prob_sum
            cross_arrive_state[cross_id]["time"] += optional_road["prob"] * optional_road["expectTime"]
        cross_with_optional_road[cross_id] = optional_road_list
    return cross_with_optional_road
  
def cal_cross_turn_ratio(cross, from_road_id, to_road_id):
    ind = 0
    #print("cross = ", cross)
    for road_id in cross:
        #print("road_id = ", road_id)
        #print("from_road_id = ", from_road_id)
        if road_id == from_road_id:
            break
        ind += 1
    if cross[(ind + 2) % 4] == to_road_id:
        return 2
    else:
        return 1
    

def dfs_random_find_path_by_dis(car, cross_id, cross_arrive_time, path_cross_id, path_road_id, path_road_through_time, crosses, roads, cross_with_from_road, road_car_situation, cross_arrive_state, cross_with_optional_road, congestion_statistics, previous_road_id):
# Dijkstra algorithm to find the cross sequence arrive_car_list which is from near to far dis for car_to_cross
# p_d = path random choose index
# ((1/dis_i)^p_d)/((1/dis_1)^p_d + (1/dis_2)^p_d + (1/dis_3)^p_d) probability to choose road_i
    #print("path_cross_id = ", path_cross_id)
    MAX_NUM = 1e8
    car_from_id = car["from"]
    car_to_id = car["to"]
    car_speed = car["speed"]
    if cross_id == car_to_id:
        path_cross_id.append(cross_id)
        return True
    # random choose path by expect run time
    # according a probability random choose next cross
    remain_capacity_sum = 0
    for optional_road in cross_with_optional_road[cross_id]:
        remain_capacity = roads[optional_road["roadToId"]]["capacity"] - np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]])
        if remain_capacity < -0.0001:
            # error situation
            remain_capacity = 0
            print("remain capacity is negative number")
            print(roads[optional_road["roadToId"]]["capacity"], np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]]))
            if cross_id != car_from_id:
                path_cross_id.pop(-1)
                path_road_id.pop(-1)
                path_road_through_time.pop(-1)
            return False
        if ifRemainCapacityRatio:
            remain_capacity_sum += remain_capacity
        else:
            remain_capacity_sum += 1
    if remain_capacity_sum < 0.0001:
        # all aptional road is Congestion
        if cross_id != car_from_id:
            path_cross_id.pop(-1)
            path_road_id.pop(-1)
            path_road_through_time.pop(-1)
        if cross_id in congestion_statistics:
            congestion_statistics[cross_id] += 1
        else:
            congestion_statistics[cross_id] = 1
        return False
    count = 0
    while True:
        count += 1
        prob_sum = 0
        for optional_road in cross_with_optional_road[cross_id]:
            remain_capacity = roads[optional_road["roadToId"]]["capacity"] - np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]])
            if remain_capacity < -0.0001:
                # error situation only ok in direct path
                remain_capacity = 0
            if not ifRemainCapacityRatio and remain_capacity > 0.0001:
                remain_capacity = 1
            prob_sum += (optional_road["prob"] * optional_road["probState"]) * (remain_capacity / remain_capacity_sum)
            #print(optional_road["prob"], remain_capacity, remain_capacity_sum)
            #print("optional_road[prob] = ",optional_road["prob"])
        #print(count, prob_sum)
        if prob_sum < 0.0001:
            # all optional road is been choose in past
            if cross_id != car_from_id:
                path_cross_id.pop(-1)
                path_road_id.pop(-1)
                path_road_through_time.pop(-1)
            if cross_id in congestion_statistics:
                congestion_statistics[cross_id] += 1
            else:
                congestion_statistics[cross_id] = 1
            return False
        random_val = random.random()
        random_val -= 0.0000001
        #print("random_val = ", random_val)
        for optional_road in cross_with_optional_road[cross_id]:
            remain_capacity = roads[optional_road["roadToId"]]["capacity"] - np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]])
            if not ifRemainCapacityRatio and remain_capacity > 0.0001:
                remain_capacity = 1
            random_val -= ((optional_road["prob"] * optional_road["probState"]) * (remain_capacity / remain_capacity_sum)) / prob_sum
            #print(optional_road["prob"], remain_capacity, remain_capacity_sum, prob_sum)
            #print("random_val = ", random_val, "raod_ratio", (optional_road["prob"] * (remain_capacity / remain_capacity_sum)) / prob_sum)
            if random_val < 0:
                path_cross_id.append(cross_id)
                path_road_id.append(optional_road["roadToId"])
                path_road_through_time.append(path_road_through_time[-1] + optional_road["throughTime"])
                ifFindPath = dfs_random_find_path_by_dis(car, optional_road["crossToId"], cross_arrive_time + optional_road["throughTime"],\
                 path_cross_id, path_road_id, path_road_through_time, crosses, roads, cross_with_from_road,\
                 road_car_situation, cross_arrive_state, cross_with_optional_road, congestion_statistics, optional_road["roadToId"])
                if ifFindPath:
                    return True
                else:
                    optional_road["probState"] = 0
                break
        

def update_road_car_situation(road_car_situation, car_start_time, path_cross_id, path_road_id, path_road_through_time):
# according car path update road_car_situation
    N = len(path_road_id)
    road_start_time = car_start_time + path_road_through_time[0]
    for i in range(N):
        road_end_time = car_start_time + path_road_through_time[i+1]
        for time in range(road_start_time, road_end_time):
            road_car_situation[path_cross_id[i]][path_road_id[i]][time] += 1
        road_start_time = road_end_time

def car_direct_to_destination(car, cross_id, cross_with_from_road, crosses, previous_road_id, path_cross_id, path_road_id, path_road_through_time):
    if cross_id == car["to"]:
        return True
    for road in cross_with_from_road[cross_id]:
        if previous_road_id == -2 or cal_cross_turn_ratio(crosses[cross_id], previous_road_id, road["id"]) == 2:
            through_time = math.ceil(road["length"] / min(car["speed"], road["speed"]))
            path_cross_id.append(cross_id)
            path_road_id.append(road["id"])
            path_road_through_time.append(path_road_through_time[-1] + through_time)
            if car_direct_to_destination(car, road["to"], cross_with_from_road, crosses, road["id"], path_cross_id, path_road_id, path_road_through_time):
                return True
            path_cross_id.pop(-1)
            path_road_id.pop(-1)
            path_road_through_time.pop(-1)
    return False

def find_shortest_path_state_update_with_road_situation(update_cross_id, cross_with_road, cross_arrive_state, car_speed, start_time, road_car_situation, roads):
# Dijkstra algorithm to update arrive cross state
    for road in cross_with_road[update_cross_id]:
        road_through_time = math.ceil(road["length"] / min(car_speed, road["speed"]))
        road_to_id = road["to"]
        cross_arrive_time = cross_arrive_state[update_cross_id]["time"]
        remain_capacity = roads[road["id"]]["capacity"] - np.max(road_car_situation[update_cross_id][road["id"]][start_time+cross_arrive_time:start_time+cross_arrive_time+road_through_time])
        if remain_capacity < -0.001:
            print("error !!!")
        if remain_capacity > 0.0001:
            if cross_arrive_state[road_to_id]["time"] > cross_arrive_state[update_cross_id]["time"] + road_through_time:
                cross_arrive_state[road_to_id]["time"] = cross_arrive_state[update_cross_id]["time"] + road_through_time
                cross_arrive_state[road_to_id]["crossFromId"] = update_cross_id
                cross_arrive_state[road_to_id]["roadFromId"] = road["id"]

def find_shortest_path_with_road_situation(car, cross_with_road, start_time, path_cross_id, path_road_id, path_road_through_time, road_car_situation, roads):
# Dijkstra algorithm to find from car["from"] to car["to"] shortest path
    MAX_TIME = 1e8
    car_from_id = car["from"]
    car_to_id = car["to"]
    car_speed = car["speed"]
    cross_arrive_state = {}
    for cross_id in cross_with_road:
        cross_arrive_state[cross_id] = {"time":MAX_TIME, "crossFromId":-1, "roadFromId":-1, "isArrive":0}
    cross_arrive_state[car_from_id] = {"time":0, "crossFromId":car_from_id, "roadFromId":0, "isArrive":1}
    find_shortest_path_state_update_with_road_situation(car_from_id, cross_with_road, cross_arrive_state, car_speed, start_time, road_car_situation, roads)
    #print("========================")
    while cross_arrive_state[car_to_id]["isArrive"] == 0:
        update_cross_id, update_cross_time = -1, MAX_TIME
        for cross_id in cross_arrive_state:
            state = cross_arrive_state[cross_id]
            if state["isArrive"] == 0:
                if state["time"] < update_cross_time:
                    update_cross_time = state["time"]
                    update_cross_id = cross_id
        if update_cross_id == -1:
            return False
        #print(update_cross_id)
        cross_arrive_state[update_cross_id]["isArrive"] = 1
        find_shortest_path_state_update_with_road_situation(update_cross_id, cross_with_road, cross_arrive_state, car_speed, start_time, road_car_situation, roads)
    if cross_arrive_state[car_to_id]["isArrive"] == 0:
        return False
    cross_id = car_to_id
    while cross_id != car_from_id:
        path_road_id.append(cross_arrive_state[cross_id]["roadFromId"])
        path_cross_id.append(cross_id)
        path_road_through_time.append(cross_arrive_state[cross_id]["time"])
        cross_id = cross_arrive_state[cross_id]["crossFromId"]
    path_cross_id.append(cross_id)
    path_road_through_time.append(cross_arrive_state[cross_id]["time"])
    
    path_road_id.reverse()
    path_cross_id.reverse()
    path_road_through_time.reverse()
    return True

def traffic_regulation(cars, crosses, roads, cross_with_to_road, cross_with_from_road):
# road car situation road_car_situation["cross_id"]["road_id"] = np.zeros(CAR_RUNNING_TIME_LENGTH)
    ans = []
    start_time = 0
    count = 0
    heap = []
    road_car_situation = {}
    for cross_id in cross_with_from_road:
        road_car_situation[cross_id] = {}
        for road in cross_with_from_road[cross_id]:
            road_car_situation[cross_id][road["id"]] = np.zeros(CAR_RUNNING_TIME_LENGTH)
    car_arrive_cross_list, car_cross_arrive_state, car_cross_with_optional_road = {}, {}, {}
    for car in cars:
        arrive_cross_list, cross_arrive_state = get_near_cross_sequence(car, cross_with_to_road)
        cross_with_optional_road = calc_expect_run_time_and_prob(arrive_cross_list, cross_arrive_state, cross_with_from_road, car["speed"])
        car_arrive_cross_list[car["id"]] = arrive_cross_list
        car_cross_arrive_state[car["id"]] = cross_arrive_state
        car_cross_with_optional_road[car["id"]] = cross_with_optional_road
    
    cars.sort(key = lambda x:-car_cross_arrive_state[x["id"]][x["from"]]["time"])
    print(car_cross_arrive_state[cars[0]["id"]][cars[0]["from"]]["time"])
    
    """
    count = 0
    for car in cars:
        path_cross_id = []
        path_road_id = []
        path_road_through_time = [0]
        ifDirectToDestination = car_direct_to_destination(car, car["from"], cross_with_from_road, crosses, -2, path_cross_id, path_road_id, path_road_through_time)
        #print("car = ", car)
        #print("path = ", path)
        if ifDirectToDestination:
            count += 1
            wait_car_N -= 1
            car["isOnTheRoad"] = car["planTime"]
            ans.append({"carId":car["id"], "startTime":car["planTime"], "path":path_road_id})
            #update_road_car_situation(road_car_situation, car["planTime"], path_cross_id, path_road_id, path_road_through_time)
    print("cars can direct to destination = ", count)
    """
    car_N = len(cars)
    for start_car_index in range(0, car_N, car_N // epoch):
        end_car_index = min(start_car_index + (car_N // epoch), car_N)
        wait_car_N = len(cars[start_car_index: end_car_index])
        start_time = 1
        while wait_car_N > 0:
            #random.shuffle(cars)
            congestion_statistics = {}
            max_run_time = 0
            for car in cars[start_car_index: end_car_index]:
                if car["isOnTheRoad"] != -1 or start_time < car["planTime"]:
                    continue
                #print(car["id"])
                # get cross sequence
                arrive_cross_list, cross_arrive_state = car_arrive_cross_list[car["id"]], car_cross_arrive_state[car["id"]]
                cross_with_optional_road = car_cross_with_optional_road[car["id"]]
                for cross_id in cross_with_optional_road:
                    for optional_road in cross_with_optional_road[cross_id]:
                        optional_road["probState"] = 1
                # find path
                path_cross_id = []
                path_road_id = []
                path_road_through_time = [0]
                cross_id = car["from"]
                cross_arrive_time = start_time
                ifFindPath = dfs_random_find_path_by_dis(car, cross_id, cross_arrive_time, path_cross_id, path_road_id, path_road_through_time, crosses, roads, cross_with_from_road, road_car_situation, cross_arrive_state, cross_with_optional_road, congestion_statistics, -100)
                #ifFindPath = find_shortest_path_with_road_situation(car, cross_with_from_road, start_time, path_cross_id, path_road_id, path_road_through_time, road_car_situation, roads)
                if not ifFindPath:
                    continue
                update_road_car_situation(road_car_situation, start_time, path_cross_id, path_road_id, path_road_through_time)
                ans.append({"carId":car["id"], "startTime":start_time, "path":path_road_id})
                max_run_time = max(max_run_time, path_road_through_time[-1])
                car["isOnTheRoad"] = start_time
                wait_car_N -= 1
            if start_time < 10 or wait_car_N < 500:
                start_time += 1
            else:
                start_time += 20
            #for cross_id in congestion_statistics:
            #    print(cross_id, ":", congestion_statistics[cross_id], ", ")
            #print("\n")
            print("start time = ", start_time, "wait car N = ", wait_car_N, "max_run_time = ", max_run_time)
            print("start_car_index = ", start_car_index, " end_car_index = ", end_car_index)
        
#    for road_id in road_car_situation[20]:
#        road = roads[road_id]
#        print("road = ", road)
#        print("road_situation = ", np.max(road_car_situation[20][road_id]))
#        print("road_capacity = ", road["capacity"])
    return ans

def write_ans(ans, answer_path):
    ans.sort(key = lambda x:x["carId"])
    with open(answer_path,'w') as w_file:
        for car_path_info in ans:
            path_str = ",".join([str(car_path_info["carId"])] + [str(car_path_info["startTime"])] + [str(val) for val in car_path_info["path"]])
            w_file.write("(" + path_str + ")\n")

def main():
    print("???????????????")
    if len(sys.argv) != 5:
        logging.info('please input args: car_path, road_path, cross_path, answerPath')
        exit(1)

    car_path = sys.argv[1]
    road_path = sys.argv[2]
    cross_path = sys.argv[3]
    answer_path = sys.argv[4]

    logging.info("car_path is %s" % (car_path))
    logging.info("road_path is %s" % (road_path))
    logging.info("cross_path is %s" % (cross_path))
    logging.info("answer_path is %s" % (answer_path))

# to read input file
    print("???????????????")
    cars, car_attr = read_car_file(car_path)
    print("???????????????")
    crosses, crossN = read_cross_file(cross_path)
    print("???????????????")
    cross_with_to_road, cross_with_from_road, roads, road_attr = read_road_file(road_path, crosses, cars)
# process
    print("???????????????")
    ans = traffic_regulation(cars, crosses, roads, cross_with_to_road, cross_with_from_road)
# to write output file
    print("???????????????")
    write_ans(ans, answer_path)


if __name__ == "__main__":
    main()