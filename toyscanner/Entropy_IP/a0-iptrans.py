#!/usr/bin/python2

def bu0(dizhi):
    dizhi1 = dizhi.split(':')
    for i in range(0, len(dizhi1)):
        if ((len(dizhi1[i]) < 4) and (len(dizhi1[i]) > 0)):
            temp = dizhi1[i]
            que0 = 4 - len(dizhi1[i])
            temp2 = "".join('0' for i in range(0, que0))
            dizhi1[i] = temp2 + temp


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
    if len(dizhi1) >= 3:
        label = 0
        print ":: times >2"
    else:
        for i, char in enumerate(dizhi):
            if char not in ':0123456789abcdef':
                print "char value not legal"
                label = 0

    if (dizhi[len(dizhi) - 1] == ':') and (dizhi[len(dizhi) - 2] != ':'):
        label = 0
    if (dizhi[0] == ':') and (dizhi[1] != ':'):
        label = 0
        print ": position not legal"


    temp3 = dizhi.split(":::")
    if len(temp3) > 1:
        print "::: not legal"
        label = 0


    dizhi2 = dizhi.split(':')
    for i in range(0, len(dizhi2)):
        if len(dizhi2[i]) >= 5:
            print "over 4"
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
    fp = open("./seeds", "r")
    lines = fp.readlines()
    fp.close()
    seedfile=open("./Sim_experiment_downsampling3k_9k/seeds","w")
    for line in lines:
        line = line.strip()
        if legal(line):
            seedfile.write(bu0(line)+"\n")
    seedfile.close()



