# -*- coding: utf-8 -*-
from __future__ import division
import random
from random import choice

def getaddr(norseeds):
    lines = []
    with open(norseeds, 'rU') as file_to_read:
        while True:
            line = file_to_read.readline()
            if not line:
                break
            line = line.strip('\n')
	    if len(line)==32:
                lines.append(line)
    file_to_read.close()
    return lines

def getmidstring(string, start_str, end_str):
    start = string.find(start_str)
    if start >= 0:
        start += len(start_str)
        end = string.find(end_str, start)
        if end >= 0:
            return string[start:end].strip()

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

def selection(norseeds, num):
    '''
    taglist=[]
    for i in range(num):
        temp_prob = random.uniform(0.0, 1)
        for key, value in probdict.items():
            # 轮次相减
            temp_prob -= value
            if temp_prob < 0.0:
                tag = key
                break
        taglist.append(tag)
    return taglist
    '''

    addrs = norseeds
    if len(addrs)>=num:
        addrslice = random.sample(addrs, num)
        return addrslice
    else:
        addrslice = []
        roundnum = num//len(addrs)
        remainder = num%len(addrs)
        for i in range(0,roundnum):
            addrslice.extend(random.sample(addrs, len(addrs)))
        addrslice.extend(random.sample(addrs, remainder))
        return addrslice

def crossover(pop, pc):  # 基因交换
    poplen = len(pop)
    for i in range(poplen - 1):
        if (random.random() < pc):
            cpoint = random.randint(0, len(pop[0]))
            temp1 = []
            temp2 = []
            temp1.extend(pop[i][0: cpoint])
            temp1.extend(pop[i + 1][cpoint: len(pop[i])])
            temp2.extend(pop[i + 1][0: cpoint])
            temp2.extend(pop[i][cpoint: len(pop[i])])
            pop[i] = "".join(temp1)
            pop[i + 1] = "".join(temp2)

def mutation(pop, pm):  # 基因突变
    px = len(pop)
    for i in range(px):
        if (random.random() < pm):
            mpoint = random.randint(0, 31)
            pointvalue = random.randint(0, 15)
            tmp = list(pop[i])
            tmp[mpoint] = num_to_string(pointvalue)
            pop[i] = ''.join(tmp)

def ga(popsize,pc,pm,norseeds):

    #popsize = 50  # Population size to be produced
    #pc = 0.8  # 个体交叉的概率
    #pm = 0.001  # 基因突变的概率
    #norseeds = "./Sim_experiment_downsampling3k_9k/s7_allresults"


    # Random choose addresses
    '''
    lines = getaddr(norseeds)
    tagdict = dict()
    addrnum = 0   
    for line in lines:
        addr = getmidstring(line, "", "    ")
        tag = getmidstring(line, "    ", "\n")
        tagdict.setdefault(tag, []).append(addr)
        addrnum += 1

    tagprobdic = dict()
    for key, value in tagdict.items():
        tagprobdic[key] = len(value) / addrnum
    
    taglist = selection(tagprobdic, popsize)
    pop = []
    for i in taglist:
        pop.append(choice(tagdict[i]))
    '''
    pop = selection(norseeds,popsize)
    crossover(pop, pc)  # 交叉繁殖
    mutation(pop, pm)  # 基因突变
    return pop

if __name__ == '__main__':
    popsize = 50  # Population size to be produced
    pc = 0.8  # 个体交叉的概率
    pm = 1  # 基因突变的概率
    norseeds = "./addrpool@1k"
    pop=ga(popsize,pc,pm,norseeds)
 
    pop=sorted(pop)
    print pop
