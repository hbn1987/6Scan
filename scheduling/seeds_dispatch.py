import math
import os
import paramiko
import threading
import time
from scp import SCPClient

def bu0(dizhi):
    dizhi1 = dizhi.split(':')
    for i in range(0, len(dizhi1)):
        # 小段地址补0 如 :AB: 补成:00AB:
        if ((len(dizhi1[i]) < 4) and (len(dizhi1[i]) > 0)):
            temp = dizhi1[i]
            # 需补0数 que0
            que0 = 4 - len(dizhi1[i])
            temp2 = "".join('0' for i in range(0, que0))
            dizhi1[i] = temp2 + temp

    # 补 ::中的0
    # count 为补完:中0后长度
    count = 0
    for i in range(0, len(dizhi1)):
        count = count + len(dizhi1[i])
    count = 32 - count
    aa = []
    aa = ''.join('0' for i in range(0, count))
    for i in range(1, len(dizhi1) - 1):
        if len(dizhi1[i]) == 0:
            dizhi1[i] = aa
    for i in range(len(dizhi1)):
        bb = ''.join(sttt for sttt in dizhi1)
    return bb

def legal(dizhi):
    dizhi1 = dizhi.split('::')
    label = 1

    # 使用::不能大于2次
    if len(dizhi1) >= 3:
        label = 0
        print(":: times >2")
    else:
        # 字符范围应为 0~9 A~F
        for i, char in enumerate(dizhi):
            if char not in ':0123456789abcdef':
                print("char value not legal:", char)
                label = 0
    # :不能出现在末位 同时允许::在最后
    # :不能出现在首位 同时允许::在最前
    if (dizhi[len(dizhi) - 1] == ':') and (dizhi[len(dizhi) - 2] != ':'):
        label = 0
    if (dizhi[0] == ':') and (dizhi[1] != ':'):
        label = 0
        print(": position not legal")

    # 不能出现 :::
    temp3 = dizhi.split(":::")
    if len(temp3) > 1:
        print("::: not legal")
        label = 0

    # 每小节位数应不大于4
    dizhi2 = dizhi.split(':')
    for i in range(0, len(dizhi2)):
        if len(dizhi2[i]) >= 5:
            print("每小节位数应不大于4")
            label = 0

    if label == 0:
        print("Error")
    return label

def iptrans(line):
    line = line.strip()
    if legal(line):
        out = bu0(line)
    return out

def iplisttrans(ipl):
    addrs = []
    for line in ipl:
        line = line.strip()
        if legal(line):
            out = bu0(line)
            addrs.append(out)
    return addrs

def retrans(lines):
    colons=[]
    for line in lines:
        lout=list(line)
        for i in range(4,35,5):
            lout.insert(i,":")
        lout="".join(lout)
        colons.append(lout)
    return colons

def file_local_to_remote(host, username, password, port = 22, local_path = './output/seeds/', remote_path = '~/dabing/HDMap6/download/'):    
        # 实例化SSHClient
        ssh_client = paramiko.SSHClient()
        # 自动添加策略，保存服务器的主机名和密钥信息，如果不添加，那么不再本地know_hosts文件中记录的主机将无法连接
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())   
        # 连接SSH服务端，以用户名和密码进行认证
        ssh_client.connect(host, port, username, password) 
    
        # 传输文件/文件夹
        scpclient = SCPClient(ssh_client.get_transport(), socket_timeout=15.0)
        local_file = local_path + host
        remote_file = remote_path + 'hitlist_' + time.strftime("%Y%m%d", time.localtime())

        # 执行cmd里的命令,stdout为正确输出，stderr为错误输出
        cmd = "mkdir -p %s"%remote_path
        stdin, stdout, stderr = ssh_client.exec_command(cmd)
        error = stderr.read().decode()
        if error != "":
            print(error)

        try:
            scpclient.put(local_file, remote_file, True)
        except FileNotFoundError :
            print("上传失败:" + local_file)
        else:
            print("上传成功:" + local_file)   

        local_file = './download/aliased_prefixes_2022128'
        try:
            scpclient.put(local_file, remote_path, True)
        except FileNotFoundError :
            print("上传失败:" + local_file)
        else:
            print("上传成功:" + local_file)
        
        ssh_client.close()

def multithread(host_list):
     thread_list = []
     for (ip,username,password) in host_list:
         thread = threading.Thread(target = file_local_to_remote, args = (ip,username,password))
         thread_list.append(thread)
     for t in thread_list:
         t.start()
     for t in thread_list:
         t.join()

def longest_match_split(iplist, seedfile):
    seeds = open(seedfile, "r").readlines()[1:]
    print("Seed number:", len(seeds))
    part_len = 10000000
    part = math.ceil(len(seeds)/part_len)
    if part > 1:
        for i in range(0,part):
            seeds_part = iplisttrans(seeds[i*part_len:(i+1)*part_len])
            
            segment_len = math.ceil(len(seeds_part)/len(iplist))

            ipdict = dict()
            for ip in iplist:
                ipdict.update({ip:[]})

            for seed in seeds_part:
                for i in range(0, len(iplist)):
                    if seed <= iplist[i] and len(ipdict[iplist[i]]) < segment_len:
                        ipdict[iplist[i]].append(seed)
                        break
                    if i == len(iplist) - 1:
                        for j in range(i , -1, -1):
                            if len(ipdict[iplist[j]]) < segment_len:
                                ipdict[iplist[j]].append(seed)
                                break

            path = './output/seeds/'
            folder = os.path.exists(path)
            if not folder:                  
                os.makedirs(path)

            for host, ips in ipdict.items():
                file_name = './output/seeds/' + retrans([host])[0]
                ips = [seed + '\n' for seed in retrans(ips)]
                f = open(file_name, "a")
                f.writelines(ips)
                f.close() 

def shortest_match_split(iplist):
    change_list = [(1,9), (9,2), (2,8), (8,3), (3,7), (7,4), (4,6), (6,5), (5,1)]
    for (source, destination) in change_list:
        print("from", iplist[source - 1], 'to', iplist[destination - 1])

def get80prefix(seedfile):
    seeds = open(seedfile, "r").readlines()[1:]
    print("Seed number:", len(seeds))
    part_len = 10000000
    part = math.ceil(len(seeds)/part_len)
    target_file = './download/hitlist_%s'%time.strftime("%Y%m%d", time.localtime())
    f = open(target_file, "a")
    if part > 1:
        for i in range(0,part):
            prefix_dict = dict()
            seeds_part = iplisttrans(seeds[i*part_len:(i+1)*part_len])
            
            for seed in seeds_part:
                prefix = seed[0:20]                
                prefix_dict[prefix] = seed
            
            for ip in prefix_dict.values():
                ip = retrans([ip])[0] + '\n'               
                f.write(ip)
    f.close() 



 
if __name__ == "__main__":
    
    local_host = '***'
    iplist = ['***', '2604:a880:400:d0::238d:3001', '2604:a880:400:d0::239c:2001', '2604:a880:800:10::129:e001', \
        '2604:a880:4:1d0::3ce:b000', '2400:6180:0:d0::4f7:e001', '2a03:b0c0:3:d0::1397:e001', '2604:a880:cad:d0::e9d:4001', '2400:6180:100:d0::a54:5001']
    seedfile = './download/hitlist_20220317'

    # get80prefix(seedfile)
    
    iplist = iplisttrans(iplist)
    iplist.sort() 

    longest_match_split(iplist, seedfile) 
    # shortest_match_split(retrans(iplist)) # rename

    remote_host_list = [(retrans([ip])[0], 'root', 'pwd') for ip in iplist if ip != iptrans(local_host)]
    multithread(remote_host_list)

    local_file = './output/seeds/' + retrans([iptrans(local_host)])[0]
    target_file = './download/hitlist_%s'%time.strftime("%Y%m%d", time.localtime())
    cmd = 'cp %s %s'%(local_file, target_file)
    os.system(cmd)