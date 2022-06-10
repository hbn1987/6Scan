import pandas as pd
import pytricia
import requests
import json

def Maxmind_geoid(file_name):
    data = pd.read_csv('./analysis/data/GeoLite2-Country-Blocks-IPv6.csv')
    pyt = pytricia.PyTricia(128)
    for _, line in data.iterrows():
        pyt[line["network"]] = line["geoname_id"]

    unknow_list = list()
    geo_data = {}
    with open(file_name) as f:
        seeds = f.read().splitlines()
        for ip in seeds:
            geo = pyt.get(ip)
            if not geo:
                unknow_list.append(ip)  
                continue
            if geo not in geo_data.keys():
                geo_data[geo] = 0
            geo_data[geo] += 1
    return geo_data, unknow_list, len(seeds)

def get(ip):
    endpoint = f'https://ipinfo.io/{ip}/json'
    response = requests.get(endpoint, verify = True)

    if response.status_code != 200:
        return False

    data = response.json()

    return data['country']

if __name__ == "__main__":
    
    file_name = "./output/hitlist_total_202269"
    geo_data, unknow_list, total = Maxmind_geoid(file_name)
    # print(geo_data, len(unknow_list))
    geo_cc_dict = dict()
    geo_cn_dict = dict()
    data = pd.read_csv('./analysis/data/GeoLite2-Country-Locations-en.csv')
    for _, line in data.iterrows():
        geo_cc_dict[line["geoname_id"]] = line["country_iso_code"]
        geo_cn_dict[line["country_iso_code"]] = line["country_name"]

    cc_data = dict()
    for k ,v in geo_data.items():
        cc_data[geo_cc_dict[k]] = v
    print("by Maxmind:", cc_data, "Unknow:", len(unknow_list))

    sample = 200
    sample_data = dict()
    for ip in unknow_list[0 : sample]:           
        cc = get(ip)
        if cc and cc not in sample_data.keys():
            sample_data[cc] = 0
        sample_data[cc] += 1
    print("by IPINFO:", sample_data)

    for k, v in sample_data.items():
        cc_data[k] += v / sum(sample_data.values()) * len(unknow_list)

    cc_data = sorted(cc_data.items(), key=lambda x: x[1], reverse=True)
    cc_data_top = dict([cc_data[i] for i in range(10)])
    print("Total country:", len(cc_data))
    for k, v in cc_data_top.items():
        print(geo_cn_dict[k], v, v/total)