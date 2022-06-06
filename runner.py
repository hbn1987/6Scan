import os
import time

def heuristic_scan():
    json_files = os.listdir("./download")
    for f in json_files:
        if f.find('.json') != -1:
            index = f.find('_')
            cc = f[index+1 : index+3]
            command = f"./6scan -A country_%s -t ICMP6 -I eth0"%cc
            print(command)
            # os.system(command)
            # time.sleep(3)

if __name__ == "__main__":
    heuristic_scan()
    
    # os.system(command)
    # time.sleep(5)
    # command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Gen -d -F ./output/seeds_ICMP6_2022418"
    # os.system(command)
    # time.sleep(5)
    # command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Hit -d -F ./output/seeds_ICMP6_2022418"
    # os.system(command)
    # time.sleep(5)
    # command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Tree -d -F ./output/seeds_ICMP6_2022418"
    # os.system(command)
    # time.sleep(5)