from __future__ import division
from treelib import Node, Tree
import os
import re


class Nodex(object):
    def __init__(self, scale, siminfo, dimension=32, qvalue=0):
        self.scale = scale
        self.siminfo = siminfo
        self.dimension = dimension
        self.qvalue = qvalue


class Addrtree(Tree):
    def __init__(self,input,output):
        Tree.__init__(self)

        calltree = "./tree/tree"
        os.system(calltree+' '+input+' '+output)

        ftree = open(output)

        while True:
            line = ftree.readline()
            pattern = re.compile(r"Location: <(\d+), (\d+)>")
            match = pattern.match(line)
            while match:
                nodetag = self.getmidstring(match.group(), ", ", ">")
                #print "nodetag:",nodetag

                line = ftree.readline()
                scalestart = self.getmidstring(line, "<" ,",")
                scaleend = self.getmidstring(line, ", ", ">")
                scale = int(scaleend) - int(scalestart) + 1
                #print "scale:", scale

                line = ftree.readline()
                simnum = self.getmidstring(line, ": ", '\n')
                #print "simnum:", simnum

                siminfo = ''
                if int(simnum) > 0:
                    line = ftree.readline()
                    for i in range(int(simnum)):
                        line = ftree.readline()
                        start = self.getmidstring(line, "<", ", ")
                        end = self.getmidstring(line, ", ", ", ")
                        str = self.getmidstring(line[7:], ", ", ">")
                        for j in range(int(start) - len(siminfo)):
                            siminfo += ''.join('*')
                        siminfo += str

                #print siminfo

                line = ftree.readline()
                parent = self.getmidstring(line, ": ", "\n")
                #print "parent:", parent


                if nodetag == "1":
                    while len(siminfo) <> 32:
                        siminfo += '*'
                    dimension = siminfo.count("*")
                    self.create_node("0", "0", data=Nodex(scale, "********************************", 32, scale/32))  # Create root
                    self.create_node(nodetag, nodetag, parent="0", data=Nodex(scale, siminfo, dimension, scale/dimension)) # Create node one
                else:
                    parentsim = self[parent].data.siminfo
                    word = '\*'
                    star = [s.start() for s in re.finditer(word, parentsim)]
                    if len(siminfo) and len(siminfo) > len(star):
                        siminfo = self.multi_sub(parentsim,star,siminfo)

                    elif len(siminfo) and len(siminfo) <= len(star):
                        siminfo = self.multi_sub(parentsim,star,siminfo)
                    else:
                        siminfo = parentsim + siminfo
                    dimension = siminfo.count("*")
                    self.create_node(nodetag, nodetag, parent=parent, data=Nodex(scale, siminfo, dimension, scale/dimension))
                break
            if not line:
                break

        # Generate random node
        self.genrandnode()

        # Backtracking the tree, update scale and prob
        # self.backtracking()

    def __del__(self):
        pass


    def getmidstring(self, string, start_str, end_str):
        start = string.find(start_str)
        if start >= 0:
            start += len(start_str)
            end = string.find(end_str, start)
            if end >= 0:
                return string[start:end].strip()

    def multi_sub(self, string, p, c):
        new = []
        for s in string:
            new.append(s)
        for index, point in enumerate(p):
            if index < len(c):
                new[point] = c[index]
            else:
                break
        return ''.join(new)

    # # Backtracking the tree, update scale and prob
    # def backtracking(self):
    #     allnodes = self.all_nodes()
    #     for i in range(self.depth() - 1, -1, -1):
    #         for node in allnodes:
    #             if self.depth(node.tag) == i and self.children(node.tag):
    #                 childscalelist = [childnode.data.scale for childnode in self.children(node.tag)]
    #                 node.data.scale = sum(childscalelist)
    #
    #     for node in allnodes:
    #         if self.parent(node.tag):
    #             node.data.prob = node.data.scale / self.parent(node.tag).data.scale

    def genrandnode(self):
        allnodes = self.all_nodes()
        for node in allnodes:
            if self.children(node.tag):
                nodetag = str(self.size() + 1)
                self.create_node(nodetag, nodetag, parent=node.tag, data=Nodex(0, node.data.siminfo, node.data.dimension, 0))


if __name__ == '__main__':
    input = "./Sim_experiment_result/s1_norseeds"
    output = "./Sim_experiment_result/s2_tree"
    ad=Addrtree(input,output)
    '''
    leaves=ad.leaves()
    for leaf in leaves:
        if leaf.data.siminfo.count("*")==1:
            print leaf.data.siminfo
            ad.remove_node(leaf.tag)
    '''
    ad.show(data_property="scale")
    ad.show(data_property="qvalue")
    ad.show(data_property="dimension")