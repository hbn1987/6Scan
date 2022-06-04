# import os
# CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
# os.chdir(CURRENT_DIR)

import pyasn
import pandas as pd

asndb = pyasn.pyasn('./analysis/ipasn_20211128.dat')


# files = ["../output/seeds_ICMP6_2021622", "../output/results_6Scan_ICMP6_2021622", "../output/results_6Hit_ICMP6_2021629", \
#         "../output/results_6Tree_ICMP6_2021623", "../output/results_6Gen_ICMP6_2021624"]
# files_name = ["C_ICMPv6", "6Scan", "6Hit", "6Tree", "6Gen"]

# files = ["../output/seeds_UDP6_2021624", "../output/results_6Scan_UDP6_2021624", "../output/results_6Hit_UDP6_2021626", \
#         "../output/results_6Tree_UDP6_2021626", "../output/results_6Gen_UDP6_2021626"]
# files_name = ["C_UDP6/53", "6Scan", "6Hit", "6Tree", "6Gen"]

# files = ["../output/seeds_UDP6_2021627", "../output/results_6Scan_UDP6_2021627", "../output/results_6Hit_UDP6_2021627", \
#         "../output/results_6Tree_UDP6_2021628", "../output/results_6Gen_UDP6_2021628"]
# files_name = ["C_UDP6/443", "6Scan", "6Hit", "6Tree", "6Gen"]

# files = ["../output/seeds_TCP6_ACK_2021629", "../output/results_6Scan_TCP6_ACK_202171", "../output/results_6Hit_TCP6_ACK_202171", \
#         "../output/results_6Tree_TCP6_ACK_2021629", "../output/results_6Gen_TCP6_ACK_2021630"]
# files_name = ["C_TCP6_ACK/80", "6Scan", "6Hit", "6Tree", "6Gen"]

# files = ["../output/seeds_TCP6_SYN_2021628", "../output/results_6Scan_TCP6_SYN_2021630", "../output/results_6Hit_TCP6_SYN_202171", \
#         "../output/results_6Tree_TCP6_SYN_2021629", "../output/results_6Gen_TCP6_SYN_2021630"]
# files_name = ["C_TCP6_SYN/80", "6Scan", "6Hit", "6Tree", "6Gen"]
def AS_detection(ASN = 47610): 
    file_name = "./Raw/6Scan_all"
    as_list = list()
    others = list()
    with open(file_name) as f:
        seeds = f.read().splitlines()
        for ip in seeds:
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

files = ["output_202205/EUI64"]

file_list = []
ASN_list = []
Num_list = []

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
    for k, v in asn_data_top.items():
        asn_data_top[k] = round(v / len(seeds), 4)
    print(asn_data_top, "AS Num:", len(asn_data) - 1, "IP Num:", len(seeds))

    # if files_name[i].find("C_") == -1: # skip the seedset
    #     file_list.extend([files_name[i]]*len(asn_data_top))
    #     ASN_list.extend(list(asn_data_top.keys()))
    #     Num_list.extend(list(asn_data_top.values()))

# dataframe = pd.DataFrame({'File':file_list, 'ASN':ASN_list, 'Num':Num_list})
# dataframe.to_csv("IP2ASN.csv",index=False,sep=',')
