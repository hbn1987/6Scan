import os
import sys
sys.path.append(os.getcwd())
from analysis.APD import alias_unfile, re_APD, Prefix2AS

def download():
    command = f"./6scan -D alias"
    os.system(command)

def alias_unfile_multi(filename, n = 5):
    for i in range(n):
        alias_unfile(filename)

def alias_statistics(G, H):
    gasser_lines = open(G).readlines()
    hmap6_lines = open(H).readlines()

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

def alias_comparision(G, G_H):
    G_alias = open(G).readlines()
    G_H_alias = open(G_H).readlines()
    D = set(G_H_alias) - set(G_alias)
    print('Newly-found alias prefix number:', len(D))
    alias_list = {'large':[], 'medium':[], 'small':[]}
    for alias in D:
        lens = alias[alias.index('/')+1:-1]
        if int(lens) <= 32:
            alias_list['large'].append(alias[:-1])
            continue
        elif int(lens) <= 64:
            alias_list['medium'].append(alias[:-1])
            continue
        else:
            alias_list['small'].append(alias[:-1])
    print('# <= /32 prefixes', len(alias_list['large']), ', # <= /64 prefixes', len(alias_list['medium']), ', # <= /120 prefixes', len(alias_list['small']))
    Prefix2AS(alias_list['large'])

if __name__ == "__main__":
    # download()
    alias_unfile_multi(filename="./download/aliased_prefixes_20221212")
    # re_APD('./residue') 
    # alias_statistics(G = './download/aliased_prefixes_20221208', H = './output/alias_prefixes_20221208')
    # alias_comparision(G='./download/aliased_prefixes_20221208', G_H='./download/aliased_prefixes_20221212')
    