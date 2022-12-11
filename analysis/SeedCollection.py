import os
import pyasn
import pytricia
import math
import pandas as pd
from multiping import multi_ping
from APD import *

def alias_statistics():

    gasser = 'download/aliased_prefixes_2022128'
    hmap6 = "output/alias_total_202266"

    gasser_lines = open(gasser).readlines()
    hmap6_lines = open(hmap6).readlines()

    gasser_dict = {'large':[], 'medium':[], 'small':[]}
    hmap6_dict = {'large':[], 'medium':[], 'small':[]}

    for line in gasser_lines:
        index = line.find('/')
        prefix_len = int(line[index+1:-1])
        if prefix_len <= 32:
            gasser_dict['large'].append(line)
            continue
        elif prefix_len <= 64:
            gasser_dict['medium'].append(line)
            continue
        else:
            gasser_dict['small'].append(line)
    
    for line in hmap6_lines:
        index = line.find('/')
        prefix_len = int(line[index+1:-1])
        if prefix_len <= 32:
            hmap6_dict['large'].append(line)
            continue
        elif prefix_len <= 64:
            hmap6_dict['medium'].append(line)
            continue
        else:
            hmap6_dict['small'].append(line)
    
    print('hmap6: large', len(hmap6_dict['large']), ', medium', len(hmap6_dict['medium']), ', small', len(hmap6_dict['small']))
    print('gasser: large', len(gasser_dict['large']), ', medium', len(gasser_dict['medium']), ', small', len(gasser_dict['small']))
    print('hamp6 - gasser: large', len(set(hmap6_dict['large']) - set(gasser_dict['large'])), ', medium', \
    len(set(hmap6_dict['medium']) - set(gasser_dict['medium'])), ', small', len(set(hmap6_dict['small']) - set(gasser_dict['small'])))


def ASN_statistics(top10):
    asndb = pyasn.pyasn('./analysis/data/ipasn_20211128.dat')
            
    asn_dict = {}
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

def RIPE_geoid(file_name):
    data = pd.read_csv('./analysis/data/RIPE-Country-IPv6.csv')
    pyt = pytricia.PyTricia(128)
    for _, line in data.iterrows():
        if line["prefixes"] != "prefixes":
            pyt.insert(line["prefixes"], line["country code"])  

    top10 = {'IN':[], 'US':[], 'CN':[], 'BR':[], 'JP':[], 'DE':[], 'MX':[], 'GB':[], 'VN':[], 'FR':[], 'Others':[]}          

    with open(file_name) as f:
        seeds = f.read().splitlines()
        for ip in seeds:
            geo = pyt.get(ip)
            if geo in top10.keys():
                top10[geo].append(ip)
            else:
                top10['Others'].append(ip)
    return top10, len(seeds)

        
if __name__ == "__main__":    
    # budget_statistics()
    hitlist_alias_alert()
    # alias_statistics()    
    # # ASN_statistics_seed()
    # ASN_statistics_result()
    
    # file_name = './output/hitlist_heuristic_ICMP6_202269_sub80'
    # top10, total = RIPE_geoid(file_name)  
    # print("Total seeds:", total/1000000, 'M')  
    # for k, v in top10.items():
    #     print(k, 'number of seeds:', round(len(v)/1000,2), 'K, ratio:', round(len(v)/total*100, 2), '%')
    # ASN_statistics(top10)
