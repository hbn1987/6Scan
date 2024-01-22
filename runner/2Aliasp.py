import os
import sys
sys.path.append(os.getcwd())
from analysis.APD import alias_unfile, re_APD, Prefix2AS
import concurrent.futures

def download():
    command = f"./6scan -D alias"
    os.system(command)

def alias_unfile_multi(file_path, n = 5):
    for i in range(n):
        process_file_parallel(file_path, lines_per_chunk=100000, max_workers=4)

def process_lines(lines):
    # 在这里执行对每个块的函数操作
    # 例如，你可以解析块中的内容，进行计算，或者执行其他操作
    result = [line.upper() for line in lines]  # 这里示范将每行文本变成大写
    return result

def split_file_into_line_chunks(file_path, lines_per_chunk=100000):
    with open(file_path, 'r') as file:
        lines = []
        for line in file:
            lines.append(line)
            if len(lines) == lines_per_chunk:
                yield lines
                lines = []
        if lines:  # 处理最后一块可能不满足 lines_per_chunk 的情况
            yield lines

def process_file_parallel(file_path, lines_per_chunk=100000, max_workers=4):
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        # 获取文件行块生成器
        line_chunks = split_file_into_line_chunks(file_path, lines_per_chunk)

        # 提交每个行块的处理任务给线程池
        futures = [executor.submit(alias_unfile, lines) for lines in line_chunks]

        # 获取结果
        results = [future.result() for future in concurrent.futures.as_completed(futures)]

    with open(file_path, 'w') as file: 
        for re in results:
            for line in re:
                file.write(line + '\n')

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
    alias_unfile_multi(file_path="./download/aliased_prefixes_20231229")
    # re_APD("./download/aliased_prefixes_20231227") 
    # alias_statistics(G = './download/aliased_prefixes_20221208', H = './output/alias_prefixes_20221208')
    # alias_comparision(G='./download/aliased_prefixes_20221208', G_H='./download/aliased_prefixes_20221212')
    