import os
import time
from iso3166 import countries_by_alpha2, countries
import json
import pandas as pd
import sys
sys.path.append(os.getcwd())
from analysis.APD import APD

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

def heuristic_scan():
    json_files = os.listdir("./download")
    hitlist_files = os.listdir("./output")
    done_cc = list()
    for f in hitlist_files:
        if f.find('hitlist') != -1:
            index = f.find('_ICMP6')
            done_cc.append(f[index-2 : index])
    for f in json_files:
        if f.find('.json') != -1:
            index = f.find('_')
            cc = f[index+1 : index+3]
            if cc not in done_cc:
                command = f"./6scan -A country_%s -t ICMP6 -I eth0"%cc
                os.system(command)
                time.sleep(3)

def budget_statistics():
    file_list = list()
    budget_top10 = 0
    budget_sum = 0
    time_top10 = 0
    time_sum = 0
    top10 = {'IN':0, 'US':0, 'CN':0, 'BR':0, 'JP':0, 'DE':0, 'MX':0, 'GB':0, 'VN':0, 'FR':0}
    for filename in os.listdir('output/'):
        if filename.find('hitlist') == 0:
            file_list.append('output/' + filename)
    for filename in file_list:
        # print(filename)
        lines = open(filename, "r").readlines()
        if (len(lines) > 3):
            line = lines[-3]
            index = line.find(':')
            budget = int(line[index+1 : -1])
            budget_sum += budget

            line = lines[-1]
            index1 = line.find('Total')
            index2 = line.find('s,')
            time = float(line[index1+6 : index2])
            time_sum += time

        else:
            # print("There is no data in file:", filename) 
            pass    
        for label in top10:
            label_fix = '_' + label + '_'
            if filename.find(label_fix) != -1:
                budget_top10 += budget
                time_top10 += time
                print(label,":",budget/1000000, 'M')
    print("BUDGET top10:", budget_top10/1000000, "M, others:", (budget_sum - budget_top10)/1000000, "M, total:", budget_sum/1000000, 'M')
    print("TIME top10:", time_top10/3600, "h, others:", (time_sum - time_top10)/3600, "h, total:", time_sum/3600, 'h')

def hitlist_alias_alert(): 
    file_list = list()   
    for filename in os.listdir('output/'):
        if filename.find('hitlist_country') == 0:
            file_list.append('output/' + filename)
    for filename in file_list:
        lines = open(filename, "r").readlines()
        lines = list(set(lines))
        H_num = len(lines)

        if H_num > 500000:
            print("Alias alert in", filename, "which contains", H_num, "seeds from heuristic scanning" )
            APD(filename)           

if __name__ == "__main__":
    download()
    rm_empty()
    create_ripe_prefixes_database()
    heuristic_scan()
    budget_statistics()
    hitlist_alias_alert()