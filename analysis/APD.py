# coding=utf-8
import random
from multiping import multi_ping
import math
import pyasn
import pandas as pd

asndb = pyasn.pyasn('./analysis/data/ipasn_20221106.dat')

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
        print("Error IP:", dizhi)
    return label

def iptrans(line):
    line = line.strip()
    if legal(line):
        out = bu0(line)
        return out
    else:
        return ''

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

def num_to_string(num):
    numbers = {
        0 : "0",
        1 : "1",
        2 : "2",
        3 : "3",
        4 : "4",
        5 : "5",
        6 : "6",
        7 : "7",
        8 : "8",
        9 : "9",
        10 : "a",
        11 : "b",
        12 : "c",
        13 : "d",
        14 : "e",
        15 : "f"
    }
    return numbers.get(num, None)

def genaddr(lenth):
    rangenum=(16**lenth)-1
    ranip=random.randint(0, rangenum)
    hexip=hex(ranip)
    if hexip[-1]=='L':
        hexip=hexip[:-1]
    c=hexip[2:].zfill(lenth)
    return c

def APD(filename): # Discover missed alias-prefix from results
    lines = open(filename).readlines()
    lines = [iptrans(line[:-1]) for line in lines if line[0] != '#' and legal(line[:-1])]
    
    prefixes = list()
    ips=list()
    for lent in range(8, 26): # Traverse prefixes from 32 to 112
        total = 0
        prefix_dict = dict()
        for line in lines:
            if line[:lent] not in prefix_dict.keys():
                prefix_dict[line[:lent]] = 0
            prefix_dict[line[:lent]] += 1
        total = sum(list(prefix_dict.values()))
        prefix_tuple=zip(prefix_dict.values(),prefix_dict.keys())
        prefix_list=list(sorted(prefix_tuple, reverse=True))
        prefixes.append(prefix_list[0][1])
        print(prefix_list[0][1], prefix_list[0][0], round(prefix_list[0][0]*100/total, 2), '%')
        if len(prefix_list) >= 2:
            prefixes.append(prefix_list[1][1])
            print(prefix_list[1][1], prefix_list[1][0], round(prefix_list[1][0]*100/total, 2), '%')
        if len(prefix_list) >= 3:
            prefixes.append(prefix_list[2][1])
            print(prefix_list[2][1], prefix_list[2][0], round(prefix_list[2][0]*100/total, 2), '%')
    print("begin pinging the prefixes:")
    print(prefixes)

    for prefix in prefixes:
        ips16=[]
        for bit in range(0,16):
            pre = prefix + num_to_string(bit)
            addr = genaddr(32-len(pre))
            ip = pre + addr
            ips16.append(ip)
        ips.append(ips16)

    prefixlist=[]
    for ips16 in ips:
        responses, no_responses = multi_ping(retrans(ips16), timeout=1, retry=2)
        print('# IPs:', len(ips16), '# responses:', len(responses))
        if len(responses) > 12: #16
            res=[]
            for addr, rtt in responses.items():
                # print "%s responded in %f seconds" % (addr, rtt)
                res.append(addr)
            nor = iplisttrans(res)
            for lent in range(25,7,-1):
                prefix_set = set([line[:lent] for line in nor])
                if len(prefix_set)==1:
                    print("alias prefix:", prefix_set)
                    prefixlist.extend(list(prefix_set))
                    break

    alias = []
    for line in prefixlist:
        x=int(math.ceil(float(len(line))/4-1))
        li=list(line)
        for i in range(4, x*5, 5):
            li.insert(i, ":")
        li = "".join(li)
        ln = "::/" + str(len(line)*4)
        li = li + ln
        alias.append(li)
    if len(alias):
        print(alias)  
        alias_file = filename.replace("hitlist", "alias") 
        alias = [line+"\n" for line in alias]
        f=open(alias_file,"a")
        f.writelines(alias)
        f.close() 

def alias_unfile(filename):
    alias = [] 
    lines = open(filename).readlines()
    for i in range(0, len(lines), 150000):
        lines_part = lines[i:i+150000]

        prefix_dict = dict()    
        for lent in range(7, 31):
            prefix_dict[lent] = set()    
            
        for line in lines_part:
            if line[0]!='#':
                index = line.find('/')
                prefix_len = int(line[index+1:-1])//4
                if prefix_len > 30:
                    print('Error:', line)
                    continue
                prefix = iptrans(line[:index])[:prefix_len]
                prefix_dict[prefix_len].add(prefix)

        prefix2remove = list()
        for lent1 in range(30, 7, -1):
            for prefix2detect in prefix_dict[lent1]:
                for lent2 in range(7, lent1):
                    for prefix_father in prefix_dict[lent2]:
                        if prefix2detect.find(prefix_father) == 0:
                            prefix2remove.append(prefix2detect)
                            # print("Remove prefix:", prefix2detect, "as contains its parent prefix of:", prefix_father)
        for prefix_redundancy in set(prefix2remove):
            prefix_dict[len(prefix_redundancy)].remove(prefix_redundancy)
        
        prefix_sum = 0   
        for k, v in prefix_dict.items():
            for line in v:
                x=int(math.ceil(float(len(line))/4-1))
                y=len(line)%4
                li=list(line)
                for i in range(4, x*5, 5):
                    li.insert(i, ":")
                li = "".join(li)
                if len(line) <= 28:
                    ln = "::/" + str(len(line)*4)
                else:
                    ln = '0'*(32-len(line))+'/'+str(len(line)*4)
                li = li + ln
                alias.append(li)
            prefix_sum += len(v)
        print("Reduce", len(lines_part), "prefixes to", prefix_sum)   
    alias = [line+"\n" for line in alias]
    f=open(filename,"w")
    f.writelines(alias)
    f.close() 

def re_APD(filename):
    alias_list = open(filename).readlines()
    arrange_prefix = list()
    for prefix in alias_list:
        temp = prefix
        prefix = prefix[:prefix.index('/')].replace(':','') 
        remainder = len(prefix)%4
        if remainder:
            str1 = list()
            index = temp.find('::/')
            for i in range(4-remainder):
                str1.append('0')
                add_zero = ''.join(str1)
            temp = temp[:index] + add_zero + temp[index:]
        arrange_prefix.append(temp)

    final_arrange_prefix = arrange_prefix
    res_dict = dict()
    for prefix in arrange_prefix:
        temp = prefix
        prefix_len = math.ceil(int(prefix[prefix.index('/')+1 : -1])/4)
        prefix = prefix[:prefix.index('/')].replace(':','')    
        if (len(prefix) > prefix_len):
            prefix = prefix[:prefix_len]
        ips16=[]
        for bit in range(0,16):
            pre = prefix + num_to_string(bit)
            addr = genaddr(32-len(pre))
            ip = pre + addr
            ips16.append(ip)

        responses, no_responses = multi_ping(retrans(ips16), timeout=1, retry=2)
        print(temp[:-1], 'has', len(responses), 'responses')

        if len(responses) not in res_dict.keys():
            res_dict[len(responses)] = 0
        res_dict[len(responses)] += 1

        if len(responses) == 0: # Remove prefixes that can never be alias prefixes
            final_arrange_prefix.remove(temp)

    res_dict = sorted(res_dict.items(), key=lambda x:x[0], reverse=True)
    res_dict = dict([res_dict[i] for i in range(len(res_dict))])
    for k, v in res_dict.items():
        print('responses:', k, 'ratio:', v*100.0/len(arrange_prefix), '%')

    f = open(filename,"w")
    f.writelines(final_arrange_prefix)
    f.close() 
    
def Prefix2AS(alias_list):
    ASname_dict = dict()
    data = pd.read_csv('./analysis/data/GeoLite2-ASN-Blocks-IPv6.csv')
    for _, line in data.iterrows():
        ASname_dict[line["autonomous_system_number"]] = line["autonomous_system_organization"]
    
    asn_prefix = dict()
    for prefix in alias_list:
        temp = prefix
        prefix = prefix[:prefix.index('/')].replace(':','')     
        addr = genaddr(32-len(prefix))
        ip = prefix + addr
        lout=list(ip)
        for i in range(4,35,5):
            lout.insert(i,":")
        ip="".join(lout)             
        asn, ripe_prefix = asndb.lookup(ip)            
        if str(asn) not in asn_prefix.keys():
            asn_prefix[str(asn)] = list()
        asn_prefix[str(asn)].append(temp) 
    asn_prefix = sorted(asn_prefix.items(), key=lambda x:len(x[1]), reverse=True)
    asn_prefix_top = dict([asn_prefix[i] for i in range(15)])
    for k, v in asn_prefix_top.items(): 
        if k != 'None':       
            print(ASname_dict[int(k)], k, v)
        else:
            print('Not found the AS:', v)
    