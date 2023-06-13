import os
import sys
import time
import pyasn
import pytricia
import pandas as pd
import json
import radix
import numpy as np

sys.path.append(os.getcwd())
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
    
def pre_scan_gasser(filename):
    for p in ['ICMP6','UDP6','TCP6_ACK','TCP6_SYN']:
        command = f"./6scan -P -I eth0 -t %s -F %s -b 120"%(p, filename)
        os.system(command)
        time.sleep(3)

def pre_scan_herustic(filename):
    for p in ['UDP6','TCP6_ACK','TCP6_SYN']:
        command = f"./6scan -P -I eth0 -t %s -F %s -b 10"%(p, filename)
        os.system(command)
        time.sleep(3)

def rm_Invalid_IP(filename):
    ip_set = set()
    lines = open(filename).readlines()
    for line in lines:
        if legal(line.strip()):
            ip_set.add(line)
    print("Number:", len(ip_set))
    f = open(filename,"w")
    f.writelines(list(ip_set))
    f.close() 

def RIPE_geoid():
    data = pd.read_csv('./analysis/data/RIPE-Country-IPv6.csv')
    pyt = pytricia.PyTricia(128)
    for _, line in data.iterrows():
        if line["prefixes"] != "prefixes":
            pyt.insert(line["prefixes"], line["country code"])  
    return pyt

def seed_analysis(file_name):
    top10 = {'IN':[], 'US':[], 'CN':[], 'BR':[], 'JP':[], 'DE':[], 'MX':[], 'GB':[], 'VN':[], 'FR':[], 'Others':[]}
    pyt = RIPE_geoid()
    asndb = pyasn.pyasn('./analysis/data/ipasn_20221212.dat')

    with open(file_name) as f:
        seeds = f.read().splitlines()
        print("Total seeds:", len(seeds)/1000000, 'M')
        cc_set = set()
        for ip in seeds:
            geo = pyt.get(ip)
            if geo in top10.keys():
                top10[geo].append(ip)
            else:
                top10['Others'].append(ip)
                cc_set.add(geo)
    for k, v in top10.items():
        print(k, 'number of seeds:', round(len(v)/1000,2), 'K, ratio:', round(len(v)/len(seeds)*100, 2), '%')
    print('Total CC:', len(cc_set)+10)
    asn_dict = dict()
    for k, v in top10.items():
        asn_set = set()
        for ip in v:
            if(ip[0] != '#'):
                asn, prefix = asndb.lookup(ip)
                if asn:
                   asn_set.add("AS" + str(asn))
        asn_dict[k] = asn_set
    asn_union = set()
    for k, v in asn_dict.items():
        print(k, "number of ASes:", len(v))
        asn_union |= v
    print("Total ASes:", len(asn_union))

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
        command = f"./6scan -t TCP6_SYN -I eth0 -s %s -F ./output/seeds_TCP6_SYN_20221216 -b 117"%s
        print(command)
        os.system(command)
        time.sleep(3)

if __name__ == "__main__":
    # download()
    # remove_alias(filename)
    # pre_scan_gasser(filename = './download/hitlist_20221207')
    # pre_scan_herustic(filename = './output/hitlist_herustic_20221213')
    # rm_Invalid_IP(filename = './output/seeds_TCP6_SYN_20221216')
    # seed_analysis(file_name = './output/seeds_ICMP6_20221216')
    # get_prefix()
    # get_missing_prefixes('./output/seeds_TCP6_SYN_20221216')
    # expanding_seeds()
    running()