# coding=utf-8
import pyasn
import pandas as pd
from multiping import multi_ping

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
        lout+='\n'
        colons.append(lout)
    return colons

def ip32(line):
    a = int('0x' + line[24:26], 16)
    b = int('0x' + line[26:28], 16)
    c = int('0x' + line[28:30], 16)
    d = int('0x' + line[30:32], 16)
    return str(a)+'.'+str(b)+'.'+str(c)+'.'+str(d)

def ip64(line):
    a = int(line[16:20])
    b = int(line[20:24])
    c = int(line[24:28])
    d = int(line[28:32])
    return str(a)+'.'+str(b)+'.'+str(c)+'.'+str(d)

def extract(filename):   
    dict32 = dict()
    dict64 = dict()
    with open(filename) as f:
        lines = f.read().splitlines()
        for line in lines:
            index32 = line.find("embedded-ipv4-32")
            index64 = line.find("embedded-ipv4-64")
            if index32 > 0:
                key = line[:line.index(',')]
                value = ip32(iptrans(line[:line.index(',')]))
                dict32.update({key : value})
            if index64 > 0:
                key = line[:line.index(',')]
                value = ip64(iptrans(line[:line.index(',')]))
                dict64.update({key : value})
        info = lines[-7:-1]
    f.close()
    return dict32, dict64, info 

def AS_consistency(ipdict):
    del_key = list()
    for ipv6, ipv4 in ipdict.items():
        asn6, prefix = asndb.lookup(ipv6)
        asn4, prefix = asndb.lookup(ipv4) 
        if asn6 != asn4:
            del_key.append(ipv6)
    for key in del_key:
        del ipdict[key]

def ping_able(ipdict):
    ip_list = list(ipdict.values())
    n = 5000
    ping_able_list = list()
    for addrs in [ip_list[i:i + n] for i in range(0, len(ip_list), n)]:
            try:
                responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
            except:
                pass
            ping_able_list += list(responses.keys())
    return ping_able_list

if __name__ == "__main__":
    filename = './AS12322'
    dict32, dict64, info = extract(filename)
    for line in info:
        print(line)
    ping32_list = ping_able(dict32)
    ping64_list = ping_able(dict64)
    print('The Active Embed-IPv4: (-32)', len(ping32_list), '(-64)', len(ping64_list), '(total)', \
    len(ping32_list) + len(ping64_list))
    AS_consistency(dict32)
    AS_consistency(dict64)    
    print('The AS-consistency Embed-IPv4: (-32)', len(dict32), '(-64)', len(dict64), '(total)', \
    len(dict32) + len(dict64))
    num32 = len((set(ping32_list) & set(dict32.values())))
    num64 = len((set(ping64_list) & set(dict64.values())))
    print('The Active and AS-consistency Embed-IPv4: (-32)', num32, '(-64)', num64, '(total)', num32+num64)
    
    
    
    
