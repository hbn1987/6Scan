import pandas as pd
import pytricia
import requests
import json
from iso3166 import countries

def RIPE_geoid(file_name):
    data = pd.read_csv('./analysis/data/RIPE-Country-IPv6.csv')
    pyt = pytricia.PyTricia(128)
    for _, line in data.iterrows():
        if line["prefixes"] != "prefixes":
            pyt.insert(line["prefixes"], line["country code"])            

    unknow_list = list()
    geo_data = {}
    with open(file_name) as f:
        seeds = f.read().splitlines()
        for ip in seeds:
            if ip[0] != '#':
                geo = pyt.get(ip)
                if not geo:
                    unknow_list.append(ip)  
                    continue
                if geo not in geo_data.keys():
                    geo_data[geo] = 0
                geo_data[geo] += 1
    return geo_data, unknow_list, len(seeds)

if __name__ == "__main__":
    
    file_name = './output/EUI64'
    cc_data, unknow_list, total = RIPE_geoid(file_name)    

    cc_data = sorted(cc_data.items(), key=lambda x: x[1], reverse=True)
    cc_data_top = dict([cc_data[i] for i in range(10)])
    print("Total country:", len(cc_data))
    for k, v in cc_data_top.items():
        print(countries.get(k).name, v, round(v/total*100,2), '%')