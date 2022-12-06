# coding=utf-8
import sys


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
        print ":: times >2"
    else:

        # 字符范围应为 0~9 A~F
        for i, char in enumerate(dizhi):
            if char not in ':0123456789abcdef':
                print "char value not legal"
                label = 0
    # :不能出现在末位 同时允许::在最后
    # :不能出现在首位 同时允许::在最前
    if (dizhi[len(dizhi) - 1] == ':') and (dizhi[len(dizhi) - 2] != ':'):
        label = 0
    if (dizhi[0] == ':') and (dizhi[1] != ':'):
        label = 0
        print ": position not legal"

    # 不能出现 :::
    temp3 = dizhi.split(":::")
    if len(temp3) > 1:
        print "::: not legal"
        label = 0

    # 每小节位数应不大于4
    dizhi2 = dizhi.split(':')
    for i in range(0, len(dizhi2)):
        if len(dizhi2[i]) >= 5:
            print "每小节位数应不大于4"
            label = 0

    if label == 0:
        print "Error"
    return label

def iptrans(fname):
    addrs = []
    fp = open(fname, "r")
    lines = fp.readlines()
    fp.close()
    for line in lines:
        line = line.strip()
        if legal(line):
            out = bu0(line)
            addrs.append(out)
    return addrs

def iplisttrans(ipl):
    addrs = []
    for line in ipl:
        #line = line.strip()
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


if __name__ == "__main__":
    fp = open("../PicExp/sim/Sim_experiment_result/biasdseeds", "r")
    lines = fp.readlines()
    fp.close()
    colons = retrans(lines)
    fp = open("../PicExp/sim/Sim_experiment_result/biasdseeds", "w")
    fp.writelines(colons)
    fp.close()


