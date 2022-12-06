# import os
# CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
# os.chdir(CURRENT_DIR)

import pyasn
import pandas as pd

asndb = pyasn.pyasn('./analysis/data/ipasn_20211128.dat')

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

def AS_statistics(files):
    ASname_dict = dict()
    data = pd.read_csv('./analysis/data/GeoLite2-ASN-Blocks-IPv6.csv')
    for _, line in data.iterrows():
        ASname_dict[line["autonomous_system_number"]] = line["autonomous_system_organization"]

    for i in range(len(files)):
        asn_data = {}
        asn_data_top = {}
        with open(files[i]) as f:
            seeds = f.read().splitlines()
            print("Adreess number:", len(seeds))
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

        asn_data = sorted(asn_data.items(), key=lambda x: x[1], reverse=True)
        asn_data_top = dict([asn_data[i] for i in range(10)])
        asn_data_top["Other"] = len(seeds) - sum(asn_data_top.values())

        print("AS Num:", len(asn_data) - 1, "IP Num:", len(seeds))
        for k, v in asn_data_top.items():
            if k != 'Other':
                print(ASname_dict[int(k[2:])], k[2:], round(v/len(seeds)*100, 2), '%', v)

if __name__ == "__main__":
    ASN = 47610
    file_name = './output/seeds_UDP6_2022418'
    AS_detection(ASN, file_name)
    
    # files = ["./output_202204/seeds_ICMP6_2022418","./output_202204/seeds_UDP6_2022418"]
    # AS_statistics(files)
