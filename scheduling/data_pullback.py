import math
import os
import paramiko
import threading
import time
from scp import SCPClient

def file_remote_to_local(host, username, password, port = 22, local_path = './data/', remote_path = '~/dabing/HDMap6/output'):    
        # 实例化SSHClient
        ssh_client = paramiko.SSHClient()
        # 自动添加策略，保存服务器的主机名和密钥信息，如果不添加，那么不再本地know_hosts文件中记录的主机将无法连接
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())   
        # 连接SSH服务端，以用户名和密码进行认证
        ssh_client.connect(host, port, username, password) 
    
        # 传输文件/文件夹
        scpclient = SCPClient(ssh_client.get_transport(), socket_timeout=15.0)
        local_path = local_path + host + '/'

        # 执行cmd里的命令,stdout为正确输出，stderr为错误输出
        cmd = "mkdir -p %s"%local_path
        os.system(cmd)        

        try:
            # scpclient.put(local_file, remote_file, True)
            scpclient.get(remote_path, local_path, recursive=True)
        except FileNotFoundError :
            print("下载失败:", remote_path, ', from host:', host)
        else:
            print("下载成功:" + remote_path, ', from host:', host)
        
        ssh_client.close()

def multithread(host_list):
     thread_list = []
     for (ip,username,password) in host_list:
         thread = threading.Thread(target = file_remote_to_local, args = (ip,username,password))
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
    multithread(host_list)
    