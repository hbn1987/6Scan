from __future__ import division
import os, re
from copy import deepcopy
from iptrans import iptrans, retrans
from multiping import multi_ping
import logging

clusterList =[]

'''
Cluster Range Function
'''
def updateClusterRange(oldRange,newRange):
    if(oldRange==""):
        return newRange
    newRange = list(newRange)
    for num in range(32):
        if(oldRange[num] != newRange[num]):
            newRange[num]='?'
    return ''.join(newRange)

def isClusterRange(seed,clusterRange):
    for num in range(32):
        if(clusterRange[num]=='?'):
            continue
        if(seed[num]!=clusterRange[num]):
            return False
    return True

def getClusterDistance(seed,cluster):
    count =0
    for num in range(32):
        if(cluster.range[num] == '?'):
            continue
        if(cluster.range[num] != seed[num]):
            count+=1
    return count

def FindCandidateSeeds(cluster,seedList):
    '''
    Computes the minimum Hamming distance between cluster.range
    and all seeds in seedList not already in cluster,
    and returns the list of seeds that are this minimum distance away
    '''
    minRangeSize = float('inf')
    seeds=[]
    for seed in seedList:
        if seed in cluster.seedSet:
            continue
        num =getClusterDistance(seed,cluster)
        if num < minRangeSize:
            minRangeSize =num
            seeds= []
            seeds.append(seed)  
        elif num == minRangeSize:
            seeds.append(seed)
    return seeds

def GrowCluster(seedList):
    '''
    Consider growing all clusters by candidate seeds,
    and select the growth resulting in the highest seed density
    and smallest cluster range size.
    '''
    maxDensity = 0
    maxIndex = 0
    maxRangeSize = float('inf')#Infinity
    maxCluster = None
    for index in range(0,len(clusterList)):
        cluster = clusterList[index]
        candidateSeeds = FindCandidateSeeds(cluster,seedList)
        for seed in candidateSeeds:
            tmpCluster = deepcopy(cluster)
            tmpCluster.addSeedUpdateRange(seed)
            for otherSeed in candidateSeeds:
                if otherSeed<>seed and isClusterRange(otherSeed,tmpCluster.range):
                    tmpCluster.addSeedUpdateRange(otherSeed)
            if(tmpCluster.rangeSize!=0):
                newDensity = len(tmpCluster.seedSet)/(16**tmpCluster.rangeSize)
            else:
                newDensity =0
            if(newDensity > maxDensity or (newDensity==maxDensity and tmpCluster.rangeSize<maxRangeSize)):
                maxDensity = newDensity
                maxIndex = index
                maxRangeSize = tmpCluster.rangeSize
                maxCluster = tmpCluster
    return maxIndex,maxCluster

class Cluster(object):
    #range = ''
    #rangeSize =0
    #seedSet=[]
    def __init__(self):
        self.range = ''
        self.rangeSize =0
        self.seedSet=[]
    def addSeedUpdateRange(self,seed):
        if seed in self.seedSet:
            return
        self.seedSet.append(seed)
        if(self.range==""):
            self.range = seed
        self.rangeSize =0
        rangeList = list(self.range)
        for num in range(32):
            if(rangeList[num]=="?"):
                self.rangeSize += 1# size +1
                continue
            if(rangeList[num] != seed[num]):
                rangeList[num]='?'
                self.rangeSize += 1# size +1
        self.range = ''.join(rangeList)
        #print "upate seed:"+str(seed)+"\n"+str(self.range)+"\n"+str(self.rangeSize)+"\n"

def InitClusters(seedList):
    for seed in seedList:
        cluster = Cluster()
        cluster.addSeedUpdateRange(seed)
        clusterList.append(cluster)

def Ipv6Gen(seedList,budgetLimit):
    '''
    Grow clusters until the sum of cluster range sizes exceeds the budget.
    For simplicity, we elide here details about handling cluster overlap
    and final cluster growth sampling to use up the budget exactly.
    '''
    InitClusters(seedList)
    budgetUsed = 0
    while True:
        growIndex,growCluster = GrowCluster(seedList)
        oldRangeSize = clusterList[growIndex].rangeSize
        newRangeSize = growCluster.rangeSize
        budgetCost = 16**newRangeSize - 16**oldRangeSize
        budgetUsed = budgetUsed + budgetCost
        if((budgetUsed<=budgetLimit) and (len(seedList)>len(growCluster.seedSet))):
            clusterList[growIndex] = growCluster
        else:
            return clusterList
def multi_sub(string, p, c):
    new = []
    for s in string:
        new.append(s)
    for index, point in enumerate(p):
        new[point] = c[index]
    return ''.join(new)

def geniplist(clulist):
    poslist=[]
    for each_clu in clulist:
        pos=[i.start() for i in re.finditer("\?",each_clu)]
        poslist.append(pos)
    posdict=dict(zip(clulist, poslist))

    outlist = []
    for key,value in posdict.items():
        rangenum=16**len(value)
        for i in range(0,rangenum):
            c=hex(i)[2:].zfill(len(value))
            outlist.append(multi_sub(key,value,c))
    return outlist

if __name__ == "__main__":

    seedList = []
    budgetLimit =250000 #budgetlimit
    input = "./seeds@r1.1"
    norseeds = iptrans(input)
    print "begin clustering"
    clusterList = Ipv6Gen(norseeds,budgetLimit)

    if not os.path.isdir(os.path.abspath(os.path.dirname(__file__))+"/Sim_experiment_downsampling3k_9k"):
        os.makedirs(os.path.abspath(os.path.dirname(__file__))+"/Sim_experiment_downsampling3k_9k")
    logging.basicConfig(filename='./Sim_experiment_downsampling3k_9k/run.log', level=logging.INFO)

    qu=[]
    for cluster in clusterList:
        if len(cluster.seedSet) <=1:
            continue
        if cluster.range in qu:
            continue
        qu.append(cluster.range)
    print "begin IP generation"
    ips = geniplist(qu)
    colons = retrans(ips)
    n = 5000
    print "begin pinging ..."
    resfile = open("./Sim_experiment_downsampling3k_9k/allresult", "a")
    for addrs in [colons[i:i + n] for i in range(0, len(colons), n)]:
        try:
            responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
        except:
            pass
        logging.info("%d responses in 5000 addresses"%len(responses))
        for addr, rtt in responses.items():
            # print "%s responded in %f seconds" % (addr, rtt)
            resfile.write(addr + "\n")
    resfile.close()
    ipfile = open("./Sim_experiment_downsampling3k_9k/targets", "w")
    for ip in ips:
        ipfile.write(ip + "\n")
    ipfile.close()


