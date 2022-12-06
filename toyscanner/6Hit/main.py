# -*- coding: utf-8 -*-
from __future__ import division
import os
import re
import random
from addrtree import Addrtree
from ga import ga
from multiping import multi_ping
import datetime
import logging
import gc
from iptrans import iptrans, iplisttrans, retrans
import math


def multi_sub(string, p, c):
    new = []
    for s in string:
        new.append(s)
    for index, point in enumerate(p):
        new[point] = c[index]
    return ''.join(new)

def genip(ip):
    pos=[i.start() for i in re.finditer("\*",ip)]
    rangenum=(16**len(pos))-1
    ranip=random.randint(0, rangenum)
    hexip=hex(ranip)
    if hexip[-1]=='L':
        hexip=hexip[:-1]
    c=hexip[2:].zfill(len(pos))
    return multi_sub(ip,pos,c)

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

def genleafip(ip):
    newiplist=[]
    for i in range(0,16):
        newip = ip.replace("*",num_to_string(i))
        newiplist.append(newip)
    return newiplist


if __name__ == "__main__":

    alladdress=[]
    allresult=[]

    # Output sorted seed with no colons
    if not os.path.exists("../PicExp/sim/Sim_experiment_result"):
        os.makedirs("../PicExp/sim/Sim_experiment_result")
    input = "./Sim_experiment_result/biasdseeds"
    output = "./Sim_experiment_result/s1_norseeds"
    norseeds = iptrans(input)
    seednum = len(norseeds)  # Gets the number of seeds
    fp = open("../PicExp/sim/Sim_experiment_result/s1_norseeds", "w")
    linesText = [norseed+"\n" for norseed in norseeds]
    fp.writelines(linesText)
    fp.close()

    logging.basicConfig(filename='../PicExp/sim/Sim_experiment_result/run.log', level=logging.INFO)
    logging.info("Output sorted seed with no colons")

    # Build address tree
    input = "./Sim_experiment_result/s1_norseeds"
    output = "./Sim_experiment_result/s2_tree"
    ad = Addrtree(input,output)
    adleavenum = len(ad.leaves())
    #ad.show()
    #ad.show(data_property="siminfo")
    logging.info("Build address tree")

    while adleavenum:

        addresses = []
        results = []

        # Nodes Chipping
        leaves = ad.leaves()
        for leaf in leaves:
            if leaf.data.qvalue >= pow(16, leaf.data.dimension-1)/leaf.data.dimension:
                leafips = genleafip(leaf.data.siminfo)
                for ip in leafips:
                    addresses.append(ip)
                ad.remove_node(leaf.tag)
        # Backtracking after pruning
        # ad.backtracking()
        #ad.show(data_property="scale")
        #ad.show(data_property="siminfo")

        # Generate probe addresses, the number of targets generated is r(=3) times of seeds number per iteration
        r = 3
        #tr = Travel(ad, r * seednum)
        leaves = ad.leaves()
        totalleaves = len(leaves)
        boltzmann = [math.exp(leaf.data.qvalue) for leaf in leaves]
        boltzmanns = sum(boltzmann)
        pvalue = [bolt/boltzmanns for bolt in boltzmann]
        gennum = [int(r*seednum*p) for p in pvalue]

        #ants = tr.search_path()
        addrtagdict = dict()
        for i in range(0, len(leaves)):
            leafgenno = gennum[i]
            while leafgenno:
                ipkey = genip(leaves[i].data.siminfo)
                addrtagdict[ipkey] = leaves[i].tag
                leafgenno = leafgenno - 1
        for key in addrtagdict.keys():# Remove duplicate IP addresses
            addresses.append(key)

        #addnodenum = len(list(set(addrtagdict.values())))
        addrinter = set(addresses) & set(alladdress)
        addresses = list(set(addresses)-addrinter)

        if len(addresses)==0 or len(set(addrtagdict.values())) <= totalleaves/20:# No new address generated
            logging.info("No new address generated, Too Convergence! Genetic Algorithm!")
            # Genetic Algorithm
            popsize = seednum  # Population size to be produced
            pc = 0.6  # 个体交叉的概率
            pm = 0.01  # 基因突变的概率
            pop=ga(popsize,pc,pm,allresult)
            pop=sorted(pop)

            # Re-build address tree
            input = "./Sim_experiment_result/s1_norseeds"
            with open(input, 'w') as f:
                for addr in pop:
                    f.write(addr + "\n")
            f.close()
            output = "./Sim_experiment_result/s2_tree"
            del ad
            gc.collect()
            ad = Addrtree(input, output)
            adleavenum = len(ad.leaves())
            continue

        # Generate the IPv6 with the colon format
        colons = retrans(addresses)

        '''
        # Scanning with zmapv6
        print "Step 4: Scanning With Zmapv6"
        target = "./Sim_experiment_downsampling3k_9k/targets"
        source = "2001:da8:d008:888d::11"
        bandwidth = "50M"
        Sim_experiment_downsampling3k_9k ="./Sim_experiment_downsampling3k_9k/results"
        cooldown = "4"
        sh = r'/usr/local/sbin/zmap'
        para = r"  --probe-module=icmp6_echoscan --ipv6-target-file=" + target + " --ipv6-source-ip=" + source +\
               " --bandwidth=" + bandwidth + " --Sim_experiment_downsampling3k_9k-file=" + Sim_experiment_downsampling3k_9k + " --cooldown-time=" + cooldown
        os.system(sh + para)
        print "Setp 4: Finished"
        '''

        # Scan with icmpv6
        logging.info("Scan with icmpv6")
        n = 5000
        responseaddrlist = []
        for addrs in [colons[i:i + n] for i in range(0, len(colons), n)]:
            try:
                responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
            except:
                pass
            for addr, rtt in responses.items():
                #print "%s responded in %f seconds" % (addr, rtt)
                results.append(addr)

        allresult += iplisttrans(results)
        alladdress += addresses
        # finish condition
        if len(alladdress) > 10000000:  # Number of probes reached
            logging.info("Number of probes reached!")
            logging.info("%d ipv6 probes had sent at %s" % (len(alladdress), nowTime))
            logging.info("%d active addresses had found at %s" % (len(allresult), nowTime))
            input = "./Sim_experiment_downsampling3k_9k/s3_alladdress"
            with open(input, 'w') as f:
                for addr in alladdress:
                    f.write(addr + "\n")
            f.close()
            input = "./Sim_experiment_downsampling3k_9k/s4_allresult"
            with open(input, 'w') as f:
                for addr in allresult:
                    f.write(addr + "\n")
            f.close()
            os._exit(0)
        nowTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        logging.info("%d ipv6 probes had sent at %s" % (len(alladdress), nowTime))
        logging.info("%d active addresses had found at %s" % (len(allresult), nowTime))

        if len(results)==0:
            logging.info("No alive address, Genetic Algorithm!")
            # Genetic Algorithm
            popsize = seednum  # Population size to be produced
            pc = 0.6  # 个体交叉的概率
            pm = 0.01  # 基因突变的概率
            pop = ga(popsize, pc, pm, allresult)
            pop = sorted(pop)
            # Re-build address tree
            input = "./Sim_experiment_result/s1_norseeds"# need fix
            with open(input, 'w') as f:
                for addr in pop:
                    f.write(addr + "\n")
            f.close()
            output = "./Sim_experiment_result/s2_tree"
            del ad
            gc.collect()
            ad = Addrtree(input, output)
            adleavenum = len(ad.leaves())
            continue

        # Get the tag in the active address
        linesnorres = iplisttrans(results)
        taglist = [leaf.tag for leaf in ad.leaves()]
        tagdict = dict(zip(taglist, [0 for i in range(0,len(taglist))]))
        for linenorres in linesnorres:
            tagkey = addrtagdict[linenorres]
            tagdict[tagkey] += 1
        for dupip in addrinter:
            tagkey = addrtagdict[dupip]
            tagdict[tagkey] -= 1


        # Update pheromone, trace back from the leaves
        logging.info("Update Q-value")
        alpha = 0.1 # The learning rate
        for key, value in tagdict.items():
            ad[key].data.qvalue = (1-alpha) * ad[key].data.qvalue + value/ad[key].data.dimension
        #ad.backtracking()
        #ad.show(data_property="scale")
