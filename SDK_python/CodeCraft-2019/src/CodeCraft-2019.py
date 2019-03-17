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

path_random_choose_index = 1
max_running_car_num = 500
CAR_RUNNING_TIME_LENGTH = 2000

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
            cars.append(car)
    cars.sort(key = lambda x:x["planTime"])
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

def read_road_file(road_path, crosses):
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
            capacity = road["channel"] * (1 + max(0, road["length"] - 2 * road["speed"]))
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
    

def random_find_path_by_dis(car, start_time, roads, cross_with_to_road, cross_with_from_road, road_car_situation):
# Dijkstra algorithm to find the cross sequence arrive_car_list which is from near to far dis for car_to_cross
# p_d = path random choose index
# ((1/dis_i)^p_d)/((1/dis_1)^p_d + (1/dis_2)^p_d + (1/dis_3)^p_d) probability to choose road_i
    MAX_NUM = 1e8
    car_from_id = car["from"]
    car_to_id = car["to"]
    car_speed = car["speed"]
    # get cross sequence
    arrive_cross_list, cross_arrive_state = get_near_cross_sequence(car, cross_with_to_road)
    cross_with_optional_road = calc_expect_run_time_and_prob(arrive_cross_list, cross_arrive_state, cross_with_from_road, car_speed)
    # random choose path by expect run time
    run_time = cross_arrive_state[car_to_id]["time"]
    path_cross_id = []
    path_road_id = []
    path_road_through_time = []
    cross_id = car_from_id
    cross_arrive_time = start_time
    while cross_id != car_to_id:
        # according a probability random choose next cross
        remain_capacity_sum = 0
        for optional_road in cross_with_optional_road[cross_id]:
            remain_capacity = roads[optional_road["roadToId"]]["capacity"] - np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]])
            if remain_capacity < 0:
                #print("remain capacity is negative number")
                path_cross_id.append(-1)
                return path_cross_id, path_road_id, path_road_through_time
            remain_capacity_sum += remain_capacity
        if remain_capacity_sum == 0:
            path_cross_id.append(-1)
            return path_cross_id, path_road_id, path_road_through_time
        prob_sum = 0
        for optional_road in cross_with_optional_road[cross_id]:
            remain_capacity = roads[optional_road["roadToId"]]["capacity"] - np.max(road_car_situation[cross_id][optional_road["roadToId"]][cross_arrive_time:cross_arrive_time+optional_road["throughTime"]])
            optional_road["prob"] *= (remain_capacity / remain_capacity_sum)
            prob_sum += optional_road["prob"]
        random_val = random.random()
        random_val -= 0.0000001
        for optional_road in cross_with_optional_road[cross_id]:
            optional_road["prob"] /= prob_sum
            random_val -= optional_road["prob"]
            if random_val < 0:
                cross_arrive_state[cross_id]["crossFromId"] = optional_road["crossToId"]
                cross_arrive_state[cross_id]["roadFromId"] = optional_road["roadToId"]
                cross_arrive_state[cross_id]["throughTime"] = optional_road["throughTime"]
                break
        path_cross_id.append(cross_id)
        path_road_id.append(cross_arrive_state[cross_id]["roadFromId"])
        run_time += cross_arrive_state[cross_id]["throughTime"]
        cross_arrive_time += cross_arrive_state[cross_id]["throughTime"]
        path_road_through_time.append(run_time)
        cross_id = cross_arrive_state[cross_id]["crossFromId"]
    path_cross_id.append(cross_id)
    return path_cross_id, path_road_id, path_road_through_time

def update_road_car_situation(road_car_situation, car_start_time, path_cross_id, path_road_id, path_road_through_time):
# according car path update road_car_situation
    N = len(path_road_id)
    road_start_time = car_start_time
    for i in range(N):
        road_end_time = car_start_time + path_road_through_time[i]
        for time in range(road_start_time, road_end_time):
            road_car_situation[path_cross_id[i]][path_road_id[i]][time] += 1
        road_start_time = road_end_time

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
    for car in cars:
        start_time = max(start_time, car["planTime"])
        while True:
            #print(car["id"])
            path_cross_id, path_road_id, path_road_through_time = random_find_path_by_dis(car, start_time, roads, cross_with_to_road, cross_with_from_road, road_car_situation)
            if path_cross_id[-1] == -1:
                start_time += 1
                continue
            break
        update_road_car_situation(road_car_situation, start_time, path_cross_id, path_road_id, path_road_through_time)
        ans.append({"carId":car["id"], "startTime":start_time, "path":path_road_id})
        end_time = start_time + path_road_through_time[-1]
        heapq.heappush(heap, end_time)
        count += 1
        if count >= max_running_car_num:
            count -= 1
            start_time = heapq.heappop(heap)
    for road_id in road_car_situation[20]:
        road = roads[road_id]
#        print("road = ", road)
#        print("road_situation = ", np.max(road_car_situation[20][road_id]))
#        print("road_capacity = ", road["capacity"])
    return ans

def write_ans(ans, answer_path):
    with open(answer_path,'w') as w_file:
        for car_path_info in ans:
            path_str = ",".join([str(car_path_info["carId"])] + [str(car_path_info["startTime"])] + [str(val) for val in car_path_info["path"]])
            w_file.write("(" + path_str + ")\n")

def main():
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
    cars, car_attr = read_car_file(car_path)
    crosses, crossN = read_cross_file(cross_path)
    cross_with_to_road, cross_with_from_road, roads, road_attr = read_road_file(road_path, crosses)
# process
    ans = traffic_regulation(cars, crosses, roads, cross_with_to_road, cross_with_from_road)
# to write output file
    write_ans(ans, answer_path)


if __name__ == "__main__":
    main()