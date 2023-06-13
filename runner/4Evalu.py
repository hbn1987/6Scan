import os
import sys
import time
import pyasn
import pytricia
import pandas as pd
import json
import radix
import numpy as np
from iso3166 import countries

sys.path.append(os.getcwd())
from analysis.APD import *
from analysis.HitRate import hitrate
from analysis.CC_statistics_RIPE import RIPE_geoid
from analysis.Legal_IP import rm_Invalid_IP
from analysis.AS_statistics_IPASN import AS_statistics, AS_similarity
from analysis.EUI64_analysis import EUI64_analysis
from analysis.EmbedIPv4_analysis import EmbedIPv4_analysis
from analysis.Entropy import draw_ent

def remove_alias(filename):
    command = f"./6scan -C %s"%filename
    os.system(command)

def APD(filename): # Discover missed alias-prefix from results
    lines = open(filename).readlines()
    lines = [iptrans(line[:-1]) for line in lines if line[0] != '#' and legal(line[:-1])]
    
    prefixes = list()
    ips=list()
    for lent in range(8, 26): # Traverse prefixes from 32 to 112
        total = 0
        prefix_dict = dict()
        for line in lines:
            if line[:lent] not in prefix_dict.keys():
                prefix_dict[line[:lent]] = 0
            prefix_dict[line[:lent]] += 1
        total = sum(list(prefix_dict.values()))
        prefix_tuple=zip(prefix_dict.values(),prefix_dict.keys())
        prefix_list=list(sorted(prefix_tuple, reverse=True))
        prefixes.append(prefix_list[0][1])
        print(prefix_list[0][1], prefix_list[0][0], round(prefix_list[0][0]*100/total, 2), '%')
        if len(prefix_list) >= 2:
            prefixes.append(prefix_list[1][1])
            print(prefix_list[1][1], prefix_list[1][0], round(prefix_list[1][0]*100/total, 2), '%')
        if len(prefix_list) >= 3:
            prefixes.append(prefix_list[2][1])
            print(prefix_list[2][1], prefix_list[2][0], round(prefix_list[2][0]*100/total, 2), '%')
    print("begin pinging the prefixes:")
    print(prefixes)

    for prefix in prefixes:
        ips16=[]
        for bit in range(0,16):
            pre = prefix + num_to_string(bit)
            addr = genaddr(32-len(pre))
            ip = pre + addr
            ips16.append(ip)
        ips.append(ips16)

    prefixlist=[]
    for ips16 in ips:
        responses, no_responses = multi_ping(retrans(ips16), timeout=1, retry=2)
        print('# IPs:', len(ips16), '# responses:', len(responses))
        if len(responses) > 12: #16
            res=[]
            for addr, rtt in responses.items():
                # print "%s responded in %f seconds" % (addr, rtt)
                res.append(addr)
            nor = iplisttrans(res)
            for lent in range(25,7,-1):
                prefix_set = set([line[:lent] for line in nor])
                if len(prefix_set)==1:
                    print("alias prefix:", prefix_set)
                    prefixlist.extend(list(prefix_set))
                    break

    alias = []
    for line in prefixlist:
        x=int(math.ceil(float(len(line))/4-1))
        li=list(line)
        for i in range(4, x*5, 5):
            li.insert(i, ":")
        li = "".join(li)
        ln = "::/" + str(len(line)*4)
        li = li + ln
        alias.append(li)
    if len(alias):
        print(alias)  

def hitrate_correction(dhc, ahc, hc, hmap6_total, scan_total, tree_total, gen_total, budget):
    for i in range(len(dhc)):
        k = list(dhc.keys())[i]
        v = dhc[k]
        if k < budget and v == 0:
            prior2_k = list(dhc.keys())[i-2]
            prior1_k = list(dhc.keys())[i-1]            
            dhc[k] = dhc[prior2_k]/2 + dhc[prior1_k]/3
            dhc[prior2_k] /= 2
            dhc[prior1_k] *= 2/3 
        if k > budget:   
            prior_k = list(ahc.keys())[i-1]
            correction = (budget - prior_k)/(k - prior_k)/10
            dhc[k] = dhc[prior_k] * correction
            dhc[prior_k] *= (1- correction)                   
            break

    for i in range(len(ahc)):
        k = list(ahc.keys())[i]
        if k > budget:
            prior_k = list(ahc.keys())[i-1]
            correction = (budget - prior_k)/(k - prior_k)
            ahc[prior_k] += ahc[k] * (1- correction)
            ahc[k] *= correction  
            break

    for i in range(len(hc)):
        k = list(hc.keys())[i]
        if k > budget:
            prior_k = list(hc.keys())[i-1]
            correction = (budget - prior_k)/(k - prior_k)
            hc[prior_k] += hc[k] * (1- correction)
            hc[k] *= correction  
            break

    print("DHC:", dhc, "sum # IPs:", sum(dhc.values()))
    print("AHC:", ahc, "sum # IPs:", sum(ahc.values()))
    print("HC:", hc, "sum # IPs:", sum(hc.values()))

    hmap6 = dict()
    scan = dict()
    tree = dict()
    gen = dict()

    total = sum(hc.values())
    base = 0
    for k, v in hc.items():
        if k < budget:
            hmap6[k] = (v+base)/total*hmap6_total/k
            base += v
        if k >= budget:
            hmap6[budget] = (v+base)/total*hmap6_total/budget
            break

    total = sum(dhc.values())
    base = 0
    for k, v in dhc.items():
        if k < budget:
            scan[k] = (v+base)/total*scan_total/k
            tree[k] = (v+base)/total*tree_total/k 
            base += v
        if k >= budget:
            scan[budget] = (v+base)/total*scan_total/budget
            tree[budget] = (v+base)/total*tree_total/budget
            break              

    total = sum(ahc.values())
    base = 0
    for k, v in ahc.items():
        if k < budget:
            gen[k] = (v+base)/total*gen_total/k
            base += v
        if k >= budget:
            gen[budget] = (v+base)/total*gen_total/budget
            break
    
    print("HMap6:", hmap6)    
    print("6Scan:", scan)
    print("6Tree:", tree)
    print("6Gen:", gen)

def re_analysis(file_name):
    top10 = {'IN':[], 'US':[], 'CN':[], 'BR':[], 'JP':[], 'DE':[], 'MX':[], 'GB':[], 'VN':[], 'FR':[], 'Others':[]}
    pyt = RIPE_geoid()
    asndb = pyasn.pyasn('./analysis/data/ipasn_20221212.dat')

    with open(file_name) as f:
        seeds = f.read().splitlines()
        print("Total seeds:", len(seeds)/1000000, 'M')
        cc_set = set()
        for ip in seeds:
            geo = pyt.get(ip)
            if geo in top10.keys():
                top10[geo].append(ip)
            else:
                top10['Others'].append(ip)
                cc_set.add(geo)
    for k, v in top10.items():
        print(k, 'number of seeds:', round(len(v)/1000,2), 'K, ratio:', round(len(v)/len(seeds)*100, 2), '%')
    print('Total CC:', len(cc_set)+10)
    asn_dict = dict()
    for k, v in top10.items():
        asn_set = set()
        for ip in v:
            if(ip[0] != '#'):
                asn, prefix = asndb.lookup(ip)
                if asn:
                   asn_set.add("AS" + str(asn))
        asn_dict[k] = asn_set
    asn_union = set()
    for k, v in asn_dict.items():
        print(k, "number of ASes:", len(v))
        asn_union |= v
    print("Total ASes:", len(asn_union))

def Country_distribution(filename):
    # rm_Invalid_IP(f)
    print('--------------------')
    print(f)
    cc_data, unknow_list, total = RIPE_geoid(f)
    LS = 0
    for k, v in cc_data.items():
        LS += np.log10(v)  
    print("Total IPs:", total, "Total country:", len(cc_data), "LS:", LS)
    cc_data = sorted(cc_data.items(), key=lambda x: x[1], reverse=True)
    cc_data_top = dict([cc_data[i] for i in range(4)])
    for k, v in cc_data_top.items():
        print(countries.get(k).name, k, round(v/total*100,2), '%')

def subprefix_analysis(file_name, lens):    
    lines = open(file_name).readlines()
    lines = iplisttrans(lines)  
    lens = int(lens/4)
    ip_dict = dict()
    for line in lines:
        k = line[:lens]
        if k in ip_dict.keys():
            ip_dict[k] += 1
        else:
            ip_dict[k] = 1

    LS = 0
    for k, v in ip_dict.items():
        LS += np.log10(v) 
    total = sum(ip_dict.values())
    print("Total IPs:", total, "Total /", lens*4, "prefixes:", len(ip_dict.keys()), "LS:", LS)
    ip_dict = sorted(ip_dict.items(), key=lambda x: x[1], reverse=True)
    ip_dict_top = dict([ip_dict[i] for i in range(4)])
    for k, v in ip_dict_top.items():
        print(k, round(v/total*100,2), '%')
    

if __name__ == "__main__":
    # for m in ['HMap6','6Scan','6Tree','6Gen']:
        # for p in ['ICMP6', 'UDP6', 'TCP6_ACK', 'TCP6_SYN']:
        #     file_name = 'output/raw_seeds_%s_%s_2023227'%(p,m)
        #     remove_alias(file_name)

    # filename = 'output/non-alias_raw_seeds_TCP6_ACK_6Gen_20221219'
    # APD(filename)
    
    # ICMP6 probe
    # file_name = './output/subspace_HMap6_ICMP6_2023228'
    # address_pool_file = './output/non-alias_raw_seeds_ICMP6_HMap6_20221218'
    # hmap6_total, scan_total, tree_total, gen_total = 84.887337, 46.228755, 44.433692, 60.487467
    # budget = 455

    # UDP6 probe
    # file_name = './output/subspace_HMap6_UDP6_202336'
    # address_pool_file = './output/non-alias_raw_seeds_UDP6_HMap6_20221218'
    # hmap6_total, scan_total, tree_total, gen_total = 4.184576, 3.542064, 3.591888, 3.916391
    # budget = 135

    # TCP6_ACK probe
    # file_name = './output/subspace_HMap6_TCP6_ACK_202337'
    # address_pool_file = './output/non-alias_raw_seeds_TCP6_ACK_HMap6_20221219'
    # hmap6_total, scan_total, tree_total, gen_total = 4.867542, 4.359037, 4.605400, 4.632192
    # budget = 656

    # TCP6_SYN probe
    # file_name = './output/subspace_HMap6_TCP6_SYN_202337'
    # address_pool_file = './output/non-alias_raw_seeds_TCP6_SYN_HMap6_20221220'
    # hmap6_total, scan_total, tree_total, gen_total = 21.916926, 16.461554, 15.437615, 19.367575
    # budget = 117
    
    # dhc, ahc, hc = hitrate(file_name, address_pool_file, 5, 1000)
    # hitrate_correction(dhc, ahc, hc, hmap6_total, scan_total, tree_total, gen_total, budget)
    # Hitrate curves are plotted in file drawing/LineChart_Hitrate.py

    # Distribution analysis. 'Cat' all the output files of a method into one file first.
    # filelist = ['output/total_HMap6_20221220',  'output/total_6Scan_20221220','output/total_6Tree_20221220',\
    #             'output/total_6Gen_2023223', 'output/total_seeds_extend', 'output/total_seeds_gasser']

    # for f in filelist:
    #     # Country_distribution(f)
    #     print('--------------------')
    #     print(f)
    #     # AS_statistics(f)
    #     subprefix_analysis(f, 32)

    # f_prior = "output/non-alias_raw_seeds_ICMP6_HMap6_20221220"
    # f_later = "output/non-alias_raw_seeds_ICMP6_HMap6_2023320"
    # asn_similarity_re = AS_similarity(f_prior, f_later)
    # folder_name = 'output/AS/'
    # for k, v in asn_similarity_re.items():
    #     file_name = folder_name + k
    #     with open(file_name, 'w') as f:
    #         for ip in v:
    #             f.write("%s\n" % ip)
    #     f.close()

    folder = 'output/AS/'
    sampling = 50

    # for f in os.listdir(folder):
    #     file_name = folder + f
    #     new_file = file_name + "_analysis"
    #     fsize = os.path.getsize(file_name)
    #     f_mb = fsize/float(1024*1024)  
    #     if f_mb < 50:        
    #         command = f"cat %s | ipv6toolkit/addr6 -i -s -q > %s"%(file_name, new_file)
    #         print(command)
    #         os.system(command)
    #     else:
    #         new_file += "_sub"            
    #         count = len(open(file_name, 'r').readlines())
    #         count = int(count / sampling)
    #         command = f"shuf -n %d %s | ipv6toolkit/addr6 -i -s -q > %s"%(count, file_name, new_file)
    #         print(command)
    #         os.system(command)

    for f in os.listdir(folder):
        if f.find("analysis") != -1:
            # print(f)
            # file_name = folder + f  
            # command = f"tail -n %d %s"%(10, file_name)
            # os.system(command)  
            # EUI64_analysis(file_name)
            # EmbedIPv4_analysis(file_name)
            print('--------------------')
        elif f.find('png') != -1:
            pass
        else:
            print(f)
            file_name = folder + f 
            draw_ent(file_name)
