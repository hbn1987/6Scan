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

sys.path.append(os.getcwd())
from analysis.APD import APD

def remove_alias(filename):
    command = f"./6scan -C %s"%filename
    os.system(command)

def loop_remove_alias(filename, aliasfile):
    remove_alias(filename)
    newfile = filename + '_non-alias'
    while(APD(newfile)):
        command = f"cat newly_detected_alias >> {aliasfile}"
        print(command)
        os.system(command)  
        remove_alias(filename)

def extract_seeds_AS_anlysis(file):
    seedset = set()
    lines = open(file).readlines()
    for line in lines:
        segs = line.strip().split(',')        
        if len(segs) == 3:
            target = segs[0].strip()
            src = segs[1].strip()
            probe = segs[2].strip()               
            seedset.add(src)
    AS_statistics(seedset)

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

def extract_hitlists(files):
    hitlist_set = set()
    for filename in files:
        print(filename)
        if filename.find('HMap6') != -1:
            lines = open(filename).readlines()
            for line in lines:
                segs = line.strip().split(',')        
                if len(segs) == 3:
                    target = segs[0].strip()
                    src = segs[1].strip()
                    probe = segs[2].strip()         
                    hitlist_set.add(src)
            continue
        else:
            lines = open(filename).readlines()
            for line in lines:
                hitlist_set.add(line.strip())  
    print(f"extract {len(hitlist_set)} addresses")  
    with open('./output/H-hitlists', 'w') as file:
        for ip in hitlist_set:
            file.write(ip + '\n')

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
    rm_Invalid_IP(filename)
    print('--------------------')
    print(filename)
    cc_data, unknow_list, total = RIPE_geoid(filename)
    LS = 0
    for k, v in cc_data.items():
        LS += np.log10(v)  
    print("Total IPs:", total, "Total country:", len(cc_data), "LS:", round(LS,2))
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
        # LS += np.log10(v) 
        LS += np.log(v) / np.log(1000)
    total = sum(ip_dict.values())
    print("Total IPs:", total, "Total /", lens*4, "prefixes:", len(ip_dict.keys()), "LS:", round(LS, 2))
    ip_dict = sorted(ip_dict.items(), key=lambda x: x[1], reverse=True)
    ip_dict_top = dict([ip_dict[i] for i in range(4)])
    for k, v in ip_dict_top.items():
        print(k, round(v/total*100,2), '%')
    

if __name__ == "__main__":
    # aliasfile='./download/aliased_prefixes_20231229'
    # for m in ['HMap6','6Scan','6Tree','6Gen']:
    #     file_name = f'output/seeds_ICMP6_{m}_2024113_probetype'
    #     print(file_name)
    #     loop_remove_alias(file_name, aliasfile)

    # for m in ['HMap6','6Scan','6Tree','6Gen']:
    #     file_name = f'output/seeds_ICMP6_{m}_2024113_probetype_non-alias'
    #     print(file_name)
    #     extract_seeds_AS_anlysis(file_name)
    
    # ICMP6 probe
    # file_name = './output/subspace_HMap6_ICMP6_2024117'
    # address_pool_file = './output/non-alias_raw_seeds_ICMP6_HMap6_20221220'
    # hmap6_total, scan_total, tree_total, gen_total = 58.796337, 31.379917, 30.161435, 41.058681
    # budget = 455

    # UDP6 probe
    # file_name = './output/subspace_HMap6_UDP6_202336'
    # address_pool_file = './output/non-alias_raw_seeds_UDP6_HMap6_20221220'
    # hmap6_total, scan_total, tree_total, gen_total = 4.184576, 3.542064, 3.591888, 3.916391
    # budget = 135

    # TCP6_ACK probe
    # file_name = './output/subspace_HMap6_TCP6_ACK_202337'
    # address_pool_file = './output/non-alias_raw_seeds_TCP6_ACK_HMap6_20221220'
    # hmap6_total, scan_total, tree_total, gen_total = 3.125036, 2.843090, 3.003775, 3.021250
    # budget = 656

    # TCP6_SYN probe
    # file_name = './output/subspace_HMap6_TCP6_SYN_202337'
    # address_pool_file = './output/non-alias_raw_seeds_TCP6_SYN_HMap6_20221220'
    # hmap6_total, scan_total, tree_total, gen_total = 20.031728, 13.375118, 12.543161, 15.736279
    # budget = 117
    
    # dhc, ahc, hc = hitrate(file_name, address_pool_file, 5, 10000)
    # hitrate_correction(dhc, ahc, hc, hmap6_total, scan_total, tree_total, gen_total, budget)
    # Hitrate curves are plotted in file drawing/LineChart_Hitrate.py

    # files = ['output/seeds_ICMP6_HMap6_2024113_probetype_non-alias', 'output/seeds_ICMP6']
    # extract_hitlists(files)

    # aliasfile='./download/aliased_prefixes_20231229'    
    # file_name = f'output/H-hitlists'
    # file_name = f'output/G-hitlists'
    # loop_remove_alias(file_name, aliasfile)

    # Distribution analysis. 
    filelist = ['output/seeds_ICMP6', 'output/H-hitlists_non-alias', 'output/G-hitlists_non-alias']
    for f in filelist:
        Country_distribution(f)
        AS_statistics(f)
        subprefix_analysis(f, 32)

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

    # for f in os.listdir(folder):
    #     if f.find("analysis") != -1:
    #         # print(f)
    #         # file_name = folder + f  
    #         # command = f"tail -n %d %s"%(10, file_name)
    #         # os.system(command)  
    #         # EUI64_analysis(file_name)
    #         # EmbedIPv4_analysis(file_name)
    #         print('--------------------')
    #     elif f.find('png') != -1:
    #         pass
    #     else:
    #         print(f)
    #         file_name = folder + f 
    #         draw_ent(file_name)
