import random
import os
from iptrans import iplisttrans, retrans
from multiping import multi_ping
import math
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



if __name__ == "__main__":

    f = open("./results")
    prefixlist=[]
    for line in f:
        prefixlist.append(line)
    f.close()

    #dir = './Sim_experiment_downsampling3k_9k/'
    #if not os.path.exists(dir):
        #os.makedirs(dir)

    ips=[]
    for lent in range(16,31): # all prefixes from 64 to 124 in 4-bit steps
        prefixs = list(set([line[:lent] for line in prefixlist]))
        #print prefixs
        for prefix in prefixs:
            ips16=[]
            for bit in range(0,16):
                pre = prefix + num_to_string(bit)
                addr = genaddr(32-len(pre))
                ip = pre + addr
                ips16.append(ip)
            ips.append(ips16)


    # Generate the IPv6 with the colon format

    colons = [retrans(ips16) for ips16 in ips]
    print "begin pinging ..."
    prefixlist=[]
    for addrs in colons:
        responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
        #print len(responses)
        if len(responses)==16:#16
            res=[]
            for addr, rtt in responses.items():
                # print "%s responded in %f seconds" % (addr, rtt)
                res.append(addr)
            nor = iplisttrans(res)
            for lent in range(31,16,-1):
                if len(set([line[:lent] for line in nor]))==1:
                    prefixlist.append(line[:lent])
                    break
    #print prefixlist
    alias = []
    for line in prefixlist:
        x=int(math.ceil(float(len(line))/4-1))
        y=len(line)%4
        li=list(line)
        for i in range(4, x*5, 5):
            li.insert(i, ":")
        li = "".join(li)
        ln = "::/" + str(len(line)*4)
        li = li + ln
        alias.append(li)
    print alias
