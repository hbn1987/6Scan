# import os
# CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
# os.chdir(CURRENT_DIR)

import pyasn
import pandas as pd
import numpy as np

asndb = pyasn.pyasn('./analysis/data/ipasn_20221212.dat')

def AS_detection(ASN, file_name): 
    as_list = list()
    others = list()
    with open(file_name) as f:
        seeds = f.read().splitlines()
        for ip in seeds:
            if ip[0] != '#':
                asn, prefix = asndb.lookup(ip)
                if asn == ASN:
                    as_list.append(ip)
                else:
                    others.append(ip)
    print(len(as_list))
    new_file_name = file_name + '_AS' + str(ASN)
    with open(new_file_name, 'w') as f:
        for ip in as_list:
            f.write("%s\n" % ip)
    f.close()

def AS_statistics(file):
    ASname_dict = dict()
    data = pd.read_csv('./analysis/data/GeoLite2-ASN-Blocks-IPv6.csv')
    for _, line in data.iterrows():
        ASname_dict[line["autonomous_system_number"]] = line["autonomous_system_organization"]

    asn_data = {}
    asn_data_top = {}
    with open(file) as f:
        seeds = f.read().splitlines()
        asn_data["Unknown"] = 0
        for ip in seeds:
            # if ip[0] != '#':
            #     index = ip.find(',')
            #     ip = ip[:index]
            asn, prefix = asndb.lookup(ip)
            if not asn:
                asn_data["Unknown"] += 1  
                continue
            if "AS" + str(asn) not in asn_data.keys():
                asn_data["AS" + str(asn)] = 0
            asn_data["AS" + str(asn)] += 1

    LS = 0
    for k, v in asn_data.items():
        LS += np.log10(v)

    print("IP Num:", len(seeds), "AS Num:", len(asn_data) - 1, 'LS:', LS)
    asn_data = sorted(asn_data.items(), key=lambda x: x[1], reverse=True)
    asn_data_top = dict([asn_data[i] for i in range(4)])
    asn_data_top["Other"] = len(seeds) - sum(asn_data_top.values())
    for k, v in asn_data_top.items():
        if k != 'Other':
            print(ASname_dict[int(k[2:])], k[2:], round(v/len(seeds)*100, 2), '%', v)

def AS_similarity(file_prior, file_later):
    ASname_dict = dict()
    data = pd.read_csv('./analysis/data/GeoLite2-ASN-Blocks-IPv6.csv')
    for _, line in data.iterrows():
        ASname_dict[line["autonomous_system_number"]] = line["autonomous_system_organization"]
    asn_data_prior = {}
    with open(file_prior) as f:
        seeds = f.read().splitlines()
        asn_data_prior["Unknown"] = set()
        for ip in seeds:
            asn, prefix = asndb.lookup(ip)
            if not asn:
                asn_data_prior["Unknown"].add(ip)  
                continue
            if "AS" + str(asn) not in asn_data_prior.keys():
                asn_data_prior["AS" + str(asn)] = set()
            asn_data_prior["AS" + str(asn)].add(ip)
    
    asn_data_later = {}
    with open(file_later) as f:
        seeds = f.read().splitlines()
        asn_data_later["Unknown"] = set()
        for ip in seeds:
            asn, prefix = asndb.lookup(ip)
            if not asn:
                asn_data_later["Unknown"].add(ip)  
                continue
            if "AS" + str(asn) not in asn_data_later.keys():
                asn_data_later["AS" + str(asn)] = set()
            asn_data_later["AS" + str(asn)].add(ip)

    asn_similarity = dict()
    for k in asn_data_prior.keys():
        if k in asn_data_later.keys():
            jaccard_distance = 1 - (len(asn_data_prior[k] & asn_data_later[k]) / len(asn_data_prior[k] | asn_data_later[k]))
            size_index = np.log10(len(asn_data_prior[k] ^ asn_data_later[k]) + 1)
            asn_similarity[k] = jaccard_distance * size_index

    asn_similarity_top = dict()
    asn_similarity_re = dict()
    asn_similarity = sorted(asn_similarity.items(), key=lambda x: x[1], reverse=True)
    asn_similarity_top = dict([asn_similarity[i] for i in range(11)])
    for k, v in asn_similarity_top.items():
        if k != "Unknown":
            print(ASname_dict[int(k[2:])], k[2:], "Scan inconsistency:", v)
            asn_similarity_re[k] = asn_data_prior[k] ^ asn_data_later[k]
    return asn_similarity_re
if __name__ == "__main__":
    ASN = 47610
    file_name = './output/seeds_UDP6_2022418'
    AS_detection(ASN, file_name)
    
    # files = "./output_202204/seeds_ICMP6_2022418"
    # AS_statistics(files)
