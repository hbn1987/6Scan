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
    json_files = os.listdir("./download/country-resource")
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
                command = f"./6scan -A country_{cc} -t ICMP6 -I eno1 -k 1 -d -U low"
                os.system(command)
                time.sleep(1)

def budget_statistics(f):
    file_list = list()
    budget_dict = dict()
    time_dict = dict()
    # top10 = {'IN':0, 'US':0, 'CN':0, 'BR':0, 'JP':0, 'DE':0, 'MX':0, 'GB':0, 'VN':0, 'FR':0}
    for filename in os.listdir(f):
        if filename.find('hitlist') == 0:
            file_list.append(f + filename)
    for filename in file_list:
        # print("read file:", filename)
        lines = open(filename, "r").readlines()

        # 逆向查找以#开头的三行
        comment_lines = []
        for i in range(len(lines)-1, -1, -1):
            line = lines[i]
            if line.startswith('#'):
                comment_lines.insert(0, line)
                if len(comment_lines) == 3:
                    break

        # 如果不是三行，则输出提示信息
        if len(comment_lines) != 3:
            print(f"文件{filename}需要重新下载，因为以'#'开头的行不是三行。")
        else:
            # print(comment_lines)

            line = comment_lines[0]
            index = line.find(':')
            budget = int(line[index+1 : -1])

            line = comment_lines[2]
            index1 = line.find('Total')
            time = float(line[index1+6 : -2])

            cc = filename.split('_country_')[1][:2]
            budget_dict[cc] = budget
            time_dict[cc] = time

    # 获取值最大的前10个键值对
    top_10_budget = sorted(budget_dict.items(), key=lambda x: x[1], reverse=True)[:10]

    # 输出结果
    print("Top 10 Budget Consuming Countries:")
    for key, value in top_10_budget:
        print(f'{key}: {round(value/1000000, 2)}M')
    total_budget = sum(budget_dict.values())
    print(f'Total Sum of Budget: {round(total_budget/1000000, 2)}M')
    total_time = sum(time_dict.values())
    print(f'Total Sum of Time: {round(total_time/3600, 2)}h')
    print(f"Average Probing Speed: {round(total_budget/total_time/1000, 2)}Kpps")

if __name__ == "__main__":
    # download()
    # rm_empty()
    # create_ripe_prefixes_database()
    # heuristic_scan() # For country-by-country scan
    budget_statistics(f = "output") # For country-by-country scan