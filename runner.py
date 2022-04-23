import os
import time

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(CURRENT_DIR)

if __name__ == "__main__":
    command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Scan -d -F ./output/seeds_ICMP6_2022418"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Gen -d -F ./output/seeds_ICMP6_2022418"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Hit -d -F ./output/seeds_ICMP6_2022418"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -t ICMP6 -I eth0 -s 6Tree -d -F ./output/seeds_ICMP6_2022418"
    os.system(command)
    time.sleep(5)