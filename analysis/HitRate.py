def subspace(file_name):
    budget_list = [10, 50, 100, 300, 600, 900, 1000]
    dhc_dict = dict()
    ahc_dict = dict()
    hc_dict = dict()
    for i in range(len(budget_list)):
        dhc_dict[budget_list[i]] = list()
        ahc_dict[budget_list[i]] = list()
        hc_dict[budget_list[i]] = list()

    dhc_count = 0
    ahc_count = 0
    hc_count = 0

    dhc_iter = 0
    ahc_iter = 0
    hc_iter = 0
    
    with open(file_name) as f:
        lines = f.read().splitlines()
        for line in lines:
            index = line.find(',')
            subspace = line[:index]
            split = line[index+2:]     
            size = pow(16, subspace.count('*'))

            exist = False
            for i in range(hc_iter + 1):
                if subspace in hc_dict[budget_list[i]]:
                    exist = True
                    break
            if not exist:
                hc_dict[budget_list[hc_iter]].append(subspace)
                hc_count += size

            if split == 'ahc':
                ahc_dict[budget_list[ahc_iter]].append(subspace)
                ahc_count += size
            else:
                dhc_dict[budget_list[dhc_iter]].append(subspace)
                dhc_count += size

            if hc_count / 1000000 >= budget_list[hc_iter]:
                hc_iter += 1
            if ahc_count / 1000000 >= budget_list[ahc_iter]:
                ahc_iter += 1
            if dhc_count / 1000000 >= budget_list[dhc_iter]:
                dhc_iter += 1                

    f.close()
    return dhc_dict, ahc_dict, hc_dict

def ip_match_subspace(ip, subspace):
    match = True
    for i in range(32):
        if subspace[i] != '*' and subspace[i] != ip[i]:
            match = False
    return match

if __name__ == "__main__":  
    file_name = './output/subspace_HMap6_TCP6_ACK_2022623'
    address_pool_file = './output/raw_HMap6_TCP6_ACK_2022622_Src_sub80'
    
    dhc_dict, ahc_dict, hc_dict = subspace(file_name)
    # print(dhc_dict, ahc_dict, hc_dict)
    
    dhc_dict_cout = dict(zip(list(dhc_dict.keys()), [0] * len(dhc_dict)))
    ahc_dict_cout = dict(zip(list(ahc_dict.keys()), [0] * len(ahc_dict)))
    hc_dict_cout = dict(zip(list(hc_dict.keys()), [0] * len(hc_dict)))

    with open(address_pool_file) as f:
        address_pool = f.read().splitlines()
        for ip in address_pool:

            for k, v in dhc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        dhc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break

            for k, v in ahc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        ahc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break

            for k, v in hc_dict.items():
                match = False
                for subspace in v:
                    if ip_match_subspace(ip, subspace):
                        hc_dict_cout[k] += 1
                        match = True
                        break
                if match:
                    break
    f.close()
    print(dhc_dict_cout, ahc_dict_cout, hc_dict_cout)