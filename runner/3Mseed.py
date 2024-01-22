import os
import sys
import time
import pyasn
import pytricia
import pandas as pd
import json
import radix # install py-radix
import numpy as np
import ipaddress
from collections import Counter
sys.path.append(os.getcwd())
from analysis.APD import APD

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
        print("Error IP:", dizhi)
    return label

def download():
    command = f"./6scan -D hitlist"
    os.system(command)

def remove_alias(filename):
    command = f"./6scan -C %s"%filename
    os.system(command)

def loop_remove_alias(filename, aliasfile):
    remove_alias(filename)
    newfile = filename + '_non-alias'
    while(APD(newfile)):
        command = f"cat newly_detected_alias >> {aliasfile}"
        print(command)
        os.system(command)  
        remove_alias(filename)
         
def pre_scan_gasser(filename):
    # for p in ['ICMP6','UDP6','TCP6_ACK','TCP6_SYN']:
        p = 'ICMP6'
        command = f"./6scan -P -I eno1 -t %s -F %s -b 120"%(p, filename)
        os.system(command)
        time.sleep(3)

def pre_scan_herustic(filename):
    for p in ['UDP6','TCP6_ACK','TCP6_SYN']:
        command = f"./6scan -P -I eth0 -t %s -F %s -b 10"%(p, filename)
        os.system(command)
        time.sleep(3)

def count_reply_type(filename):
    lines = open(filename).readlines()
    # 初始化计数器
    count0 = 0
    count1 = 0
    count3 = 0
    count129 = 0
    count129s = 0

    for line in lines:
        # 分割每行数据
        segs = line.strip().split(',')        
        if len(segs) == 3:
            count0 += 1
            target = segs[0].strip()
            src = segs[1].strip()
            probe = segs[2].strip()          
            if "DstUnreach" in probe or probe == "1":
                count1 += 1
            if "TimeExceeded" in probe or probe == "3":
                count3 += 1
            if "EchoReply" in probe or probe == "129":
                count129 += 1
                if target == src:
                    count129s += 1 

    print(f"Total: {count0}, DstUnreach: {count1}, TimeExceeded: {count3}, EchoReply: {count129}, EchoReply(Target==Src): {count129s}, Sum: {count1+count3+count129}")

def count_interfaces(filename):
    lines = open(filename).readlines()
    # 初始化计数器
    count0 = 0
    count1 = 0

    for line in lines:
        count0 += 1
        # 查找第一个逗号的位置
        first_comma_index = line.find(',')
        # 查找第二个逗号的位置
        second_comma_index = line[first_comma_index+1:].find(',')
        second_comma_index = second_comma_index + first_comma_index + 1
    
        if first_comma_index != -1 and second_comma_index != -1:
            
            target = line[:first_comma_index].strip()
            target = target.replace(' ', '')   

            src = line[first_comma_index+1:second_comma_index].strip()
            src = src.replace(' ', '')       
            
            if target != src:
                count1 += 1
    print(f"Total addresses: {count0}, Total interfaces: {count1}")

def extract_seeds(files):
    seedset = set()
    for filename in files:
        if filename.find('ping6') != -1:
            if filename.find('low') != -1:
                lines = open(filename).readlines()
                for line in lines:
                    segs = line.strip().split(',')        
                    if len(segs) == 3:
                        target = segs[0].strip()
                        src = segs[1].strip()
                        probe = segs[2].strip()          
                        # if "DstUnreach" in probe or probe == "1":
                        #     count1 += 1
                        # if "TimeExceeded" in probe or probe == "3":
                        #     count3 += 1
                        if "EchoReply" in probe or probe == "129":
                            seedset.add(src)
                print(f"Extracting {len(seedset)} seeds from ping6-low scan;")
            elif filename.find('ran') != -1:
                lines = open(filename).readlines()
                for line in lines:
                    segs = line.strip().split(',')        
                    if len(segs) == 3:
                        target = segs[0].strip()
                        src = segs[1].strip()
                        probe = segs[2].strip()          
                        if "DstUnreach" in probe or probe == "1":
                            seedset.add(src)
                        if "TimeExceeded" in probe or probe == "3":
                            seedset.add(src)
                print(f"Expanding to {len(seedset)} seeds from ping6-ran scan;")
        elif filename.find('Houtput') != -1:
            lines = open(filename).readlines()
            for line in lines:
                # 查找第一个逗号的位置
                first_comma_index = line.find(',')
                # 查找第二个逗号的位置
                second_comma_index = line[first_comma_index+1:].find(',')
                second_comma_index = second_comma_index + first_comma_index + 1
            
                if first_comma_index != -1 and second_comma_index != -1:                
                    target = line[:first_comma_index].strip()
                    target = target.replace(' ', '') 
                    src = line[first_comma_index+1:second_comma_index].strip()
                    src = src.replace(' ', '')                 
                    if target != src:
                        seedset.add(src)
            print(f"Expanding to {len(seedset)} seeds from trace6 scan.")
    
    with open('./output/seeds', 'w') as file:
        for seed in seedset:
            file.write(seed + '\n')

def extract_seeds_anlysis(files):
    for filename in files:
        seedset = set()
        print(filename)
        if filename.find('low') != -1:
            lines = open(filename).readlines()
            for line in lines:
                segs = line.strip().split(',')        
                if len(segs) == 3:
                    target = segs[0].strip()
                    src = segs[1].strip()
                    probe = segs[2].strip()          
                    # if "DstUnreach" in probe or probe == "1":
                    #     count1 += 1
                    # if "TimeExceeded" in probe or probe == "3":
                    #     count3 += 1
                    if "EchoReply" in probe or probe == "129":
                        seedset.add(src)
            seedset_analysis(seedset)
            continue
        elif filename.find('ran') != -1:
            lines = open(filename).readlines()
            for line in lines:
                segs = line.strip().split(',')        
                if len(segs) == 3:
                    target = segs[0].strip()
                    src = segs[1].strip()
                    probe = segs[2].strip()          
                    if "DstUnreach" in probe or probe == "1":
                        seedset.add(src)
                    if "TimeExceeded" in probe or probe == "3":
                        seedset.add(src)
            seedset_analysis(seedset)
            continue
        elif filename.find('output64') != -1:
            lines = open(filename).readlines()
            for line in lines:
                # 查找第一个逗号的位置
                first_comma_index = line.find(',')
                # 查找第二个逗号的位置
                second_comma_index = line[first_comma_index+1:].find(',')
                second_comma_index = second_comma_index + first_comma_index + 1
            
                if first_comma_index != -1 and second_comma_index != -1:                
                    target = line[:first_comma_index].strip()
                    target = target.replace(' ', '') 
                    src = line[first_comma_index+1:second_comma_index].strip()
                    src = src.replace(' ', '')                 
                    if target != src:
                        seedset.add(src)
            seedset_analysis(seedset)
            continue
    
    with open('./output/seeds', 'w') as file:
        for seed in seedset:
            file.write(seed + '\n')

def seedset_analysis(seeds):
    cc_dict = dict()
    asn_dict = dict()
    pyt = RIPE_geoid()
    asndb = pyasn.pyasn('./analysis/data/ipasn_20221212.dat')

    print("Total seeds:", round(len(seeds)/1000000,2), 'M')
    for ip in seeds:
        geo = pyt.get(ip)
        if geo in cc_dict.keys():
            cc_dict[geo].append(ip)
        else:
            cc_dict[geo] = [ip]

        asn, prefix = asndb.lookup(ip)
        if asn:
            if asn in asn_dict.keys():
                asn_dict[asn].append(ip)
            else:
                asn_dict[asn] = [ip]
  
    # Output top 10 CC information
    print('Total CC:', len(cc_dict))
    # top10_cc = sorted(cc_dict.items(), key=lambda x: len(x[1]), reverse=True)[:10]
    # for cc, ips in top10_cc:
    #     print(f'Top 10 CC - {cc}: number of seeds: {len(ips)}, ratio: {round(len(ips) / len(seeds) * 100, 2)}%')    

    # Output top 10 ASN information
    print("Total ASes:", len(asn_dict))
    # top10_asn = sorted(asn_dict.items(), key=lambda x: len(x[1]), reverse=True)[:10]
    # for asn, ips in top10_asn:
    #     print(f'Top 10 ASN - {asn}: number of seeds: {len(ips)}, ratio: {round(len(ips) / len(seeds) * 100, 2)}%')

def RIPE_geoid():
    data = pd.read_csv('./analysis/data/RIPE-Country-IPv6.csv')
    pyt = pytricia.PyTricia(128)
    for _, line in data.iterrows():
        if line["prefixes"] != "prefixes":
            pyt.insert(line["prefixes"], line["country code"])  
    return pyt

def seed_analysis(file_name):
    cc_dict = dict()
    asn_dict = dict()
    pyt = RIPE_geoid()
    asndb = pyasn.pyasn('./analysis/data/ipasn_20221212.dat')

    with open(file_name) as f:
        seeds = f.read().splitlines()
        print("Total seeds:", round(len(seeds)/1000000,2), 'M')
        for ip in seeds:
            geo = pyt.get(ip)
            if geo in cc_dict.keys():
                cc_dict[geo].append(ip)
            else:
                cc_dict[geo] = [ip]

            asn, prefix = asndb.lookup(ip)
            if asn:
                if asn in asn_dict.keys():
                    asn_dict[asn].append(ip)
                else:
                    asn_dict[asn] = [ip]
  
    # Output top 10 CC information
    print('Total CC:', len(cc_dict))
    top10_cc = sorted(cc_dict.items(), key=lambda x: len(x[1]), reverse=True)[:10]
    for cc, ips in top10_cc:
        print(f'Top 10 CC - {cc}: number of seeds: {len(ips)}, ratio: {round(len(ips) / len(seeds) * 100, 2)}%')    

    # Output top 10 ASN information
    print("Total ASes:", len(asn_dict))
    top10_asn = sorted(asn_dict.items(), key=lambda x: len(x[1]), reverse=True)[:10]
    for asn, ips in top10_asn:
        print(f'Top 10 ASN - {asn}: number of seeds: {len(ips)}, ratio: {round(len(ips) / len(seeds) * 100, 2)}%')

def ABCToNum(n):
    dic = {'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9,'a':10,'b':11,'c':12,'d':13,'e':14,'f':15}
    return dic[n]
def NumToABC(n):
    dic = {0:'0',1:'1',2:'2',3:'3',4:'4',5:'5',6:'6',7:'7',8:'8',9:'9',10:"a",11:"b",12:"c",13:"d",14:"e",15:"f"}
    return dic[n]

def get_prefix():
    path='download'
    kid_ipaddr=[]
    for file_name in os.listdir(path):
        if file_name.find('json') != -1:
            file_path = path + '/' + file_name
            with open(file_path, encoding='utf-8') as a:
                result=json.load(a)
                iplist=result.get('data').get('resources').get('ipv6')
                for ipaddr in iplist:
                    if int(ipaddr[-2:])==32:
                        kid_ipaddr.append(ipaddr)
                    elif int(ipaddr[-2:]) > 32:
                        if len(ipaddr)==9:
                            ipaddr = ipaddr[:-2] + '32'
                        else:
                            index = ipaddr.find(':', 5)
                            ipaddr = ipaddr[:index] + '::/32'
                        kid_ipaddr.append(ipaddr)
                    else:
                        if len(ipaddr)<=9:
                            ipaddr=ipaddr[0:4]+':0000'+ ipaddr[4:]
                        num=32-int(ipaddr[-2:])
                        if(4>=num):
                            for i in range (2**num):
                                kid_ipaddr.append(ipaddr[:-6]+NumToABC(ABCToNum(ipaddr[-6])+i)+"::/32")
                        elif(4<num<=8):
                            for i in range (2**(num-4)):
                                for j in range (16):
                                    kid_ipaddr.append(ipaddr[:-7]+NumToABC(ABCToNum(ipaddr[-7])+i)+NumToABC(j)+"::/32")
                        elif(8<num<=12):
                            for i in range (2**(num-8)):
                                for j in range (16):
                                    for k in range(16):
                                        kid_ipaddr.append(ipaddr[:-8]+NumToABC(ABCToNum(ipaddr[-8])+i)+NumToABC(j)+NumToABC(k)+"::/32")
                    
                        elif(12<num<=16):
                            for i in range (2**(num-12)):
                                for j in range (16):
                                    for k in range(16):
                                        for l in range(16):
                                            kid_ipaddr.append(ipaddr[:-9]+NumToABC(ABCToNum(ipaddr[-9])+i)+NumToABC(j)+NumToABC(k)+NumToABC(l)+"::/32")
                        else:
                            print('error:', ipaddr)
    kid_ipaddr=set(kid_ipaddr)
    print('# of /32 BGP prefixes (from RIPE):', len(kid_ipaddr))
    with open('prefixes32', 'w') as f:
        for prefix in kid_ipaddr:
            f.write(prefix +'\n')

def get_missing_prefixes(seed_file):
    ip6Rtree = radix.Radix()
    with open ('prefixes32') as f:
        prefixes = f.read().splitlines()
        for prefix in prefixes:
            rnode = ip6Rtree.add(prefix)
    prefix_dict = dict()
    new_prefixes = set()
    with open (seed_file) as f:
        seeds = f.read().splitlines()
        for seed in seeds:
            node = ip6Rtree.search_best(seed)
            if node:
                if node.prefix not in prefix_dict.keys():
                    prefix_dict[node.prefix] = 0
                prefix_dict[node.prefix] += 1
            else:
                index = seed.find(':', 5)
                prefix = seed[:index] + '::/32'
                if prefix.find(':::') != -1:
                    prefix = prefix.replace(':::', '::')
                new_prefixes.add(prefix)
    print('Adding', len(new_prefixes), '/32 prefixes to file prefixes32')
    if len(new_prefixes):
        with open('prefixes32', 'a') as f:
            for prefix in new_prefixes:
                f.write(prefix +'\n')
    prefixes = ip6Rtree.prefixes()
    prefix_no_seed = len(prefixes) - len(prefix_dict.keys())
    print('# /32 prefixes:', len(prefixes))
    print('# /32 prefixes with seeds:', len(prefix_dict.keys()))
    print('# /32 prefixes with no seeds:', prefix_no_seed)
    with open('prefixes32', 'w') as f:
        for prefix in prefixes:
            f.write(prefix +'\n')

    values = np.array(list(prefix_dict.values()))
    mean = np.mean(values)
    std = np.std(values)
    median = np.percentile(values, 50)
    upper = np.percentile(values, 90) 
    print('The mean and standard deviation of the seed number in /32 prefixes containing seed(s) is %.2f, %.2f'% (mean, std))
    print('The median of seed number in /32 prefixes containing seed(s) is %i'% median)
    print('%90 of the /32 prefixes containing seed(s) have a seed number less than', int(upper))

def expanding_seeds():
    for p in ['ICMP6', 'UDP6','TCP6_ACK','TCP6_SYN']:
        filename = 'output/seeds_%s_20221213'%p
        command = f"./6scan -E 300 -I eth0 -t %s -F %s -b 120"%(p, filename)
        os.system(command)
        time.sleep(3)

def running():
    strategy = ['6Scan', '6Tree', '6Gen']
    for s in strategy:
        command = f"./6scan -t ICMP6 -I eno1 -s %s -F ./output/seeds_ICMP6_20240113 -b 180 -d"%s
        print(command)
        os.system(command)
        time.sleep(3)


if __name__ == "__main__":
    # download()
    # pre_scan_gasser(filename = './download/hitlist_20231226')
    # pre_scan_herustic(filename = './output/hitlist_herustic_20221213')
    
    # filelist=['./output/hitlist-ping6-low', './output/hitlist-xmap-low','./output/hitlist-ping6-ran', './output/hitlist-xmap-ran']
    # filelist=['./output/Houtput64', './output/Foutput64','./output/Youtput64']
    # aliasfile='./download/aliased_prefixes_20231229'
    # for filename in filelist:
    #     print(filename)
    #     loop_remove_alias(filename, aliasfile)
    
    filelist=['./output/hitlist-ping6-low_non-alias', './output/hitlist-xmap-low_non-alias', \
              './output/hitlist-ping6-ran_non-alias', './output/hitlist-xmap-ran_non-alias']
    filelist=['./output/Houtput64_non-alias', './output/Foutput64_non-alias','./output/Youtput64_non-alias']  
    # for filename in filelist:
    #     print(filename)
        # count_reply_type(filename)
        # count_interfaces(filename)
    extract_seeds_anlysis(filelist)

    # filelist=['./output/hitlist-ping6-low_non-alias','./output/hitlist-ping6-ran_non-alias','./output/Houtput64_non-alias'] 
    # extract_seeds(filelist)
    # seedset_analysis(filename="")

    # get_prefix()
    # get_missing_prefixes('./output/seeds_TCP6_SYN_20221216')
    # expanding_seeds()
    # running()
    

