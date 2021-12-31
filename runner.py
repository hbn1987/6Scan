import os

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(CURRENT_DIR)

if __name__ == "__main__":
    command = f"{CURRENT_DIR}/6scan -t UDP6 -I eth0 -s 6Scan"
    os.system(command)
    command = f"{CURRENT_DIR}/6scan -t UDP6 -I eth0 -s 6Gen"
    os.system(command)