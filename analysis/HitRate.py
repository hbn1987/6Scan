# coding=utf-8
import random

def subspace_analysis(file_name, subspace_ratio = 100):
    budget_list = [10, 50, 100, 200, 400, 600, 800]
    dhc_dict = dict()
    ahc_dict = dict()
    hc_dict = dict()
    for i in range(len(budget_list)):
        dhc_dict[budget_list[i]] = list()
        ahc_dict[budget_list[i]] = list()
        hc_dict[budget_list[i]] = list()

    dhc_count = 0
    ahc_count = 0
    hc_count = 0

    dhc_iter = 0
    ahc_iter = 0
    hc_iter = 0

    ratio = subspace_ratio
    
    with open(file_name) as f:
        lines = f.read().splitlines()
        print('# subspaces:', len(lines), 'sampling ratio:', 1.0*100/ratio, '%', 'sampling size:', int(len(lines)/ratio))
        lines = random.sample(lines, int(len(lines)/ratio))
        for line in lines:
            index = line.find(',')
            subspace = line[:index]
            split = line[index+2:]     
            size = pow(16, subspace.count('*')) * ratio

            exist = False
            for i in range(hc_iter + 1):
                if subspace in hc_dict[budget_list[i]]:
                    exist = True
                    break
            if not exist:
                hc_dict[budget_list[hc_iter]].append(subspace)
                hc_count += size

            if split == 'ahc':
                ahc_dict[budget_list[ahc_iter]].append(subspace)
                ahc_count += size
            else:
                dhc_dict[budget_list[dhc_iter]].append(subspace)
                dhc_count += size

            if hc_count / 1000000 >= budget_list[hc_iter]:
                hc_iter += 1
            if ahc_count / 1000000 >= budget_list[ahc_iter]:
                ahc_iter += 1
            if dhc_count / 1000000 >= budget_list[dhc_iter]:
                dhc_iter += 1                

    f.close()
    return dhc_dict, ahc_dict, hc_dict

def ip_match_subspace(ip, subspace):
    match = True
    for i in range(32):
        if subspace[i] != '*' and subspace[i] != ip[i]:
            match = False
    return match

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
        print("Error IP:", dizhi)
    return label

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

def iptrans(line):
    line = line.strip()
    if legal(line):
        out = bu0(line)
        return out
    else:
        return ''
    
def hitrate(subspace_file, address_pool_file, subspace_ratio = 100, ip_ratio = 1000):
    dhc_dict, ahc_dict, hc_dict = subspace_analysis(subspace_file, subspace_ratio)
    dhc_dict_t = dict()
    ahc_dict_t = dict()
    hc_dict_t = dict()
    for k , v in dhc_dict.items():
        dhc_dict_t[k] = len(v)
    print("# DHC subspaces:", dhc_dict_t, "total:", sum(dhc_dict_t.values()))
    for k , v in ahc_dict.items():
        ahc_dict_t[k] = len(v)
    print("# AHC subspaces:", ahc_dict_t, "total:", sum(ahc_dict_t.values()))
    for k , v in hc_dict.items():
        hc_dict_t[k] = len(v)
    print("# HC subspaces:", hc_dict_t, "total:", sum(hc_dict_t.values()))
    
    dhc_dict_cout = dict(zip(list(dhc_dict.keys()), [0] * len(dhc_dict)))
    ahc_dict_cout = dict(zip(list(ahc_dict.keys()), [0] * len(ahc_dict)))
    hc_dict_cout = dict(zip(list(hc_dict.keys()), [0] * len(hc_dict)))

    with open(address_pool_file) as f:
        address_pool = f.read().splitlines()
        ratio = ip_ratio
        print('# IPs:', len(address_pool), 'sampling ratio:', 1.0*100/ratio, '%', 'sampling size:', int(len(address_pool)/ratio))
        address_pool = random.sample(address_pool, int(len(address_pool)/ratio))
        for ip in address_pool:
            ip = iptrans(ip)
            for k, v in dhc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        dhc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break

            for k, v in ahc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        ahc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break

            for k, v in hc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        hc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break
    f.close()
    # print("DHC:", dhc_dict_cout, "sum # IPs:", sum(dhc_dict_cout.values()))
    # print("AHC:", ahc_dict_cout, "sum # IPs:", sum(ahc_dict_cout.values()))
    # print("HC:", hc_dict_cout, "sum # IPs:", sum(hc_dict_cout.values()))
    return dhc_dict_cout, ahc_dict_cout, hc_dict_cout


if __name__ == "__main__":  
    file_name = './output/subspace_HMap6_ICMP6_2023228'
    address_pool_file = './output/non-alias_raw_seeds_ICMP6_HMap6_20221218'
    hitrate(file_name, address_pool_file)