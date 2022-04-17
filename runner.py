import os
import time

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(CURRENT_DIR)

if __name__ == "__main__":
    command = f"{CURRENT_DIR}/6scan -P -t ICMP6 -I eth0 -d"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -P -t UDP6 -I eth0 -d"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -P -t TCP6_SYN -I eth0 -d"
    os.system(command)
    time.sleep(5)
    command = f"{CURRENT_DIR}/6scan -P -t TCP6_ACK -I eth0 -d"
    os.system(command)