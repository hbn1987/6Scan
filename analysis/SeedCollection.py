import os
import pyasn
import pandas as pd
import pytricia

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

def budget_statistics():
    file_list = list()
    budget = 0
    budget_top10 = 0
    budget_sum = 0
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
        else:
            print("There is no data in file:", filename) 
            pass    
        for label in top10:
            label_fix = '_' + label + '_'
            if filename.find(label_fix) != -1:
                budget_top10 += budget
                print(label,":",budget/1000000,'M')
    print("top10:", budget_top10/1000000, "M, others:", (budget_sum - budget_top10)/1000000, "M, total:", budget_sum/1000000, 'M')

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
    budget_statistics()
    # alias_statistics()
    # hitlist_alias_alert()
    # # ASN_statistics_seed()
    # ASN_statistics_result()
    
    # file_name = './output/hitlist_heuristic_ICMP6_202269_sub80'
    # top10, total = RIPE_geoid(file_name)  
    # print("Total seeds:", total/1000000, 'M')  
    # for k, v in top10.items():
    #     print(k, 'number of seeds:', round(len(v)/1000,2), 'K, ratio:', round(len(v)/total*100, 2), '%')
    # ASN_statistics(top10)
