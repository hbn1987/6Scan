def probeType_analysis(file_name):
    type_dict = {'ICMP6_EchoReply_noPayload':0, 'ICMP6_EchoReply_withPayload_Target':0, 'ICMP6_EchoReply_withPayload_Src':0, \
             'ICMP6_TimeExceeded_noPayload':0, 'ICMP6_TimeExceeded_withPayload_Target':0, 'ICMP6_TimeExceeded_withPayload_Src':0, \
             'ICMP6_DstUnreach_noPayload':0, 'ICMP6_DstUnreach_withPayload_Target':0, 'ICMP6_DstUnreach_withPayload_Src':0, \
             'UDP6_noPayload':0, 'UDP6_withPayload_Target':0, 'UDP6_withPayload_Src':0, \
             'TCP6_SYN_ACK_noPayload':0, 'TCP6_SYN_ACK_withPayload_Target':0, 'TCP6_SYN_ACK_withPayload_Src':0 ,\
             'TCP6_RST_ACK_noPayload':0, 'TCP6_RST_ACK_withPayload_Target':0, 'TCP6_RST_ACK_withPayload_Src':0, \
             'TCP6_ACK_noPayload':0, 'TCP6_ACK_withPayload_Target':0, 'TCP6_ACK_withPayload_Src':0 , \
             'TCP6_RST_noPayload':0, 'TCP6_RST_withPayload_Target':0, 'TCP6_RST_withPayload_Src':0, \
             'TCP6_SYN_noPayload':0, 'TCP6_SYN_withPayload_Target':0, 'TCP6_SYN_withPayload_Src':0, 'TCP6_FIN_PUSH_ACK_noPayload':0}
    IPs = set()
    multiTypeIPs = list()
    with open(file_name) as f:
        IPs_types = f.read().splitlines()
        IPs_types = list(set(IPs_types))
        print("Reply num:", len(IPs_types))
        for ip_type in IPs_types:
            if ip_type[0] == '#':
                continue
            index = ip_type.find(',')
            ip = ip_type[0:index]     
            if ip in IPs:
                multiTypeIPs.append(ip)
            else:     
                IPs.add(ip)
                label = False
                for type in type_dict:
                    if ip_type.find(type) != -1:
                        type_dict[type] += 1
                        label = True
                        break
                if (not label):
                    print(ip_type)              

    print("Distinct IP num:", len(IPs))
    print("Check Distinct IP:", sum(type_dict.values()))  

    type_dict_frap = {'noPayload':0, 'withPayload_Target':0, 'withPayload_Src':0}

    for k, v in type_dict.items():
        if v:
            print(k, v, v/len(IPs))
            for key in type_dict_frap.keys():
                if k.find(key) != -1:
                    type_dict_frap[key] += v

    for k, v in type_dict_frap.items():
        print(k, v, v/len(IPs))

    # index = file_name.find('probetype')
    # new_file_name = file_name[0:index-1]
    # with open(new_file_name, 'w') as f:
    #     for ip in IPs:
    #         f.write("%s\n" % ip)

if __name__ == "__main__":
    file_list = ["output/seeds_ICMP6_202257_probetype", "output/seeds_ICMP6_202258_probetype", "output/seeds_ICMP6_202259_probetype", \
                 "output/seeds_ICMP6_2022510_probetype", "output/seeds_ICMP6_2022511_probetype", "output/seeds_ICMP6_2022512_probetype", \
                 "output/seeds_ICMP6_2022513_probetype", "output/seeds_ICMP6_2022514_probetype", "output/seeds_ICMP6_2022515_probetype"]
    file_list = ["output/seeds_UDP6_202257_probetype", "output/seeds_UDP6_202258_probetype", "output/seeds_UDP6_202259_probetype", \
                 "output/seeds_UDP6_2022510_probetype", "output/seeds_UDP6_2022511_probetype", "output/seeds_UDP6_2022512_probetype", \
                 "output/seeds_UDP6_2022513_probetype", "output/seeds_UDP6_2022514_probetype", "output/seeds_UDP6_2022515_probetype"]
    file_list = ["output/seeds_TCP6_ACK_202257_probetype", "output/seeds_TCP6_ACK_202258_probetype", "output/seeds_TCP6_ACK_202259_probetype", \
                 "output/seeds_TCP6_ACK_2022510_probetype", "output/seeds_TCP6_ACK_2022511_probetype", "output/seeds_TCP6_ACK_2022512_probetype", \
                 "output/seeds_TCP6_ACK_2022513_probetype", "output/seeds_TCP6_ACK_2022514_probetype", "output/seeds_TCP6_ACK_2022515_probetype"]
    for file_name in file_list:
        print(file_name)
        probeType_analysis(file_name)
        print('--------------------')
