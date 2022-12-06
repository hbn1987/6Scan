# coding=utf-8

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

def OUI_extract(OUI_file = './analysis/data/OUI'):
    OUI_manufacturer = dict()
    with open(OUI_file) as f:
        lines = f.read().splitlines()
        for line in lines:
            if line.find("base 16") != -1:
                # print(line)
                index = line.find('\t') + 1
                OUI_manufacturer[line[:6]] = line[index:]  
    return OUI_manufacturer

def extract(filename):  
    total = 0 
    dict_eui = dict()
    with open(filename) as f:
        lines = f.read().splitlines()
        for line in lines:
            index_eui = line.find("ieee-derived")
            if index_eui > 0:
                key = line.strip()[-8:]
                key = key.replace('-', '').upper()
                if key not in dict_eui.keys():
                    dict_eui[key] = 0
                dict_eui[key] += 1
    f.close()
    return dict_eui 
    
if __name__ == "__main__":
    filename = './AS47610_20221111'
    oui_data = extract(filename)
    OUI_manufacturer = OUI_extract()
    # for k , v in OUI_manufacturer.items():
    #     print(k, v)
    total = sum(list(oui_data.values()))
    print("Total address:", total, "Total OUI:", len(oui_data))
    oui_data = sorted(oui_data.items(), key=lambda x: x[1], reverse=True)
    oui_data_top = dict([oui_data[i] for i in range(10)])
    for k, v in oui_data_top.items():
        if k in OUI_manufacturer.keys():
            print(k, OUI_manufacturer[k], v, round(v/total*100, 2), '%')
        else:
            print(k, 'Unknown vendor', v, round(v/total*100,2), '%')