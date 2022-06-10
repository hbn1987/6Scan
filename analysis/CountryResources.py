from iso3166 import countries_by_alpha2, countries
import os
import time
import json
import pandas as pd

def download():
    print("Total countries: ", len(countries_by_alpha2))
    for cc in countries_by_alpha2:
        country = "country_" + cc
        command = f"./6scan -D %s"%country
        os.system(command)
        time.sleep(1)

def rm_empty():
    json_files = os.listdir("./download")
    count = 0
    for f in json_files:
        if f.find('.json') != -1:
            f = "./download/" + f
            with open(f, 'r') as jf:
                country_data = json.load(jf)
                if len(country_data['data']['resources']['ipv6']):
                    count += 1
                else:
                    rm = "rm %s"%f
                    os.system(rm)
                
    print('The number of countries with IPv6 BGP prefixes:', count)

def create_ripe_prefixes_database():
    json_files = os.listdir("./download")
    for f in json_files:
        if f.find('.json') != -1:
            index = f.find("_")
            cc = f[index+1:index+3]
            f = "./download/" + f
            with open(f, 'r') as jf:
                country_data = json.load(jf)
                prefixes = country_data['data']['resources']['ipv6']
                ccs = list()
                country_names = list()
                for i in range(len(prefixes)):
                    ccs.append(cc)
                    country_names.append(countries.get(cc).name)
                #字典中的key值即为csv中列名
                dataframe = pd.DataFrame({'prefixes' : prefixes, 'country code' : ccs, 'country name' : country_names})
                #将DataFrame存储为csv,index表示是否显示行名，default=True
                dataframe.to_csv("RIPE-Country-IPv6.csv", index=False, sep=',', mode='a')

if __name__ == "__main__":
    create_ripe_prefixes_database()

                    
