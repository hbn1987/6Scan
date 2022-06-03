import sys
import paramiko
import threading

def getConnection(ip,username,password,command,port = 22):
     ssh = paramiko.SSHClient()
     policy = paramiko.AutoAddPolicy()
     ssh.set_missing_host_key_policy(policy)
     ssh.connect(
         hostname = ip,  
         port = port,  
         username = username,  
         password = password)
     stdin, stdout, stderr = ssh.exec_command(command) 
     result = stdout.read().decode() 
     error = stderr.read().decode()
     print("[connect success] | ip : %s" % ip)
     print(result)
     if error != "":
         print(error) 
     ssh.close()

def main(host_list,command):
     thread_list = []
     for ip,username,password in host_list:
         thread = threading.Thread(target = getConnection, args = (ip,username,password,command))
         thread_list.append(thread)
     for t in thread_list:
         t.start()
     for t in thread_list:
         t.join()
 
 
if __name__ == "__main__":
    host_list = [
         ('***', 'root', 'pwd'),
         ('206.189.203.211', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('204.48.21.128', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('159.203.112.107', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('137.184.115.26', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('188.166.190.9', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('46.101.156.199', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('142.93.151.255', 'root', 'B7RBq9tof9urp1DeqjXc'),
         ('128.199.27.115', 'root', 'B7RBq9tof9urp1DeqjXc')
    ]
    command = 'python3 ~/dabing/HDMap6/runner.py'
    
    main(host_list,command)