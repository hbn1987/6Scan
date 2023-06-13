# coding=utf-8
from scipy import stats
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
plt.style.use("ggplot")
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
matplotlib.rcParams['figure.figsize'] = (5, 3)
plt.rc('font',family="Times New Roman")
matplotlib.rcParams['xtick.direction'] = 'in'


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

def entropy(filename):  
    with open(filename) as f:
        lines = f.read().splitlines()
    f.close()
    addrs = iplisttrans(lines)

    ent_dict = dict()
    for i in range(0, 32):
        ent_dict[i] = dict()
        for addr in addrs:
            if not addr[i] in ent_dict[i]:
                ent_dict[i][addr[i]] = 1
            else:
                ent_dict[i][addr[i]] += 1

    ent_res = list()
    for i in range(0, 32):
        Entropy = stats.entropy(list(ent_dict[i].values()))
        ent_res.append(Entropy)
    ent_res = [ent/max(ent_res) for ent in ent_res]
    print("Entropy:", ent_res)
    return ent_res

def draw_ent(filename):
    x = list(range(1, 33))
    y = entropy(filename)
    fig, ax = plt.subplots()
    ax.bar(x=x, height=y, width=0.95, color = 'dodgerblue', edgecolor = 'black')
    ax.set_title("Nomalized Entropy", fontsize=26, y=1)
    tick_spacing = 8
    ax.xaxis.set_major_locator(ticker.MultipleLocator(tick_spacing))
    frame = plt.gca()
    frame.axes.get_yaxis().set_visible(False)
    # frame.set_facecolor('none') # 设置图例legend背景透明
    plt.xticks([1, 8, 16, 24, 32],['1', '8', '16', '24', '32'])
    plt.xlim((1, 33))
    plt.tick_params(axis= 'both', which= 'major', labelsize= 26)
    plt.tight_layout()
    new_file = filename + '.png'
    plt.savefig(new_file)

if __name__ == "__main__":
    filename = "./output/AS/AS1136"  
    