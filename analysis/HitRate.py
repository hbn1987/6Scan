def seeds(probe):
    seed_file_name = "./output/seeds_" + probe + "_2022418"
    return seed_file_name

if __name__ == "__main__":  
    probe = "ICMP6"
    file_list = ["Raw/raw_seeds_ICMP6_6Scan_2022424_10M", "Raw/raw_seeds_ICMP6_6Gen_2022424_50M", \
                "Raw/raw_seeds_ICMP6_6Gen_2022424_100M", "Raw/raw_seeds_ICMP6_6Gen_2022424_600M", \
                "Raw/raw_seeds_ICMP6_6Gen_2022424_1000M"] 
    file_list = ["Raw/raw_seeds_ICMP6_6Hit_2022425_10M", "Raw/raw_seeds_ICMP6_6Tree_2022425_50M", \
                "Raw/raw_seeds_ICMP6_6Tree_2022425_100M", "Raw/raw_seeds_ICMP6_6Tree_2022425_600M", \
                "Raw/raw_seeds_ICMP6_6Tree_2022425_1000M"] 
    file_list = ["Raw/raw_seeds_ICMP6_6Tree_2022425_10M", "Raw/raw_seeds_ICMP6_6Hit_2022425_50M", \
                "Raw/raw_seeds_ICMP6_6Hit_2022425_100M", "Raw/raw_seeds_ICMP6_6Hit_2022425_600M", \
                "Raw/raw_seeds_ICMP6_6Hit_2022425_1000M"]
    file_list = ["Raw/raw_seeds_ICMP6_6Gen_2022424_10M", "Raw/raw_seeds_ICMP6_6Scan_2022424_50M", \
                "Raw/raw_seeds_ICMP6_6Scan_2022424_100M", "Raw/raw_seeds_ICMP6_6Scan_2022424_600M", \
                "Raw/raw_seeds_ICMP6_6Scan_2022424_1000M"] 
    file_list = ["Raw/raw_seeds_ICMP6_6Scan_2022424_1500M", "Raw/raw_seeds_ICMP6_6Hit_2022425_1500M", \
                "Raw/raw_seeds_ICMP6_6Tree_2022425_1500M", "Raw/raw_seeds_ICMP6_6Gen_2022424_1500M"]

    probe = "UDP6"
    file_list = ["Raw/raw_seeds_UDP6_6Scan_2022426_10M", "Raw/raw_seeds_UDP6_6Scan_2022426_50M", \
                "Raw/raw_seeds_UDP6_6Scan_2022426_100M", "Raw/raw_seeds_UDP6_6Gen_2022426_600M", \
                "Raw/raw_seeds_UDP6_6Gen_2022426_1000M"]
    file_list = ["Raw/raw_seeds_UDP6_6Hit_2022427_10M", "Raw/raw_seeds_UDP6_6Hit_2022427_50M", \
                "Raw/raw_seeds_UDP6_6Hit_2022427_100M", "Raw/raw_seeds_UDP6_6Hit_2022427_600M", \
                "Raw/raw_seeds_UDP6_6Hit_2022427_1000M"] 
    file_list = ["Raw/raw_seeds_UDP6_6Tree_2022426_10M", "Raw/raw_seeds_UDP6_6Tree_2022426_50M", \
                "Raw/raw_seeds_UDP6_6Tree_2022426_100M", "Raw/raw_seeds_UDP6_6Tree_2022426_600M", \
                "Raw/raw_seeds_UDP6_6Tree_2022426_1000M"]
    file_list = ["Raw/raw_seeds_UDP6_6Gen_2022426_10M", "Raw/raw_seeds_UDP6_6Gen_2022426_50M", \
                "Raw/raw_seeds_UDP6_6Gen_2022426_100M", "Raw/raw_seeds_UDP6_6Scan_2022426_600M", \
                "Raw/raw_seeds_UDP6_6Scan_2022426_1000M"]  

    file_list = ["Raw/raw_seeds_UDP6_6Scan_2022426_1500M", "Raw/raw_seeds_UDP6_6Tree_2022426_1500M", "Raw/raw_seeds_UDP6_6Gen_2022426_1500M"]

    probe = "TCP6_SYN"
    file_list = ["Raw/raw_seeds_TCP6_SYN_6Scan_2022427_10M", "Raw/raw_seeds_TCP6_SYN_6Scan_2022427_50M", \
                "Raw/raw_seeds_TCP6_SYN_6Scan_2022427_100M", "Raw/raw_seeds_TCP6_SYN_6Scan_2022427_600M", \
                "Raw/raw_seeds_TCP6_SYN_6Scan_2022427_1000M"]
    file_list = ["Raw/raw_seeds_TCP6_SYN_6Scan_2022427_1500M", "Raw/raw_seeds_TCP6_SYN_6Gen_2022428_1500M"] 

    probe = "TCP6_ACK"
    file_list = ["Raw/raw_seeds_TCP6_ACK_6Scan_2022428_10M", "Raw/raw_seeds_TCP6_ACK_6Gen_2022428_50M", \
                "Raw/raw_seeds_TCP6_ACK_6Gen_2022428_100M", "Raw/raw_seeds_TCP6_ACK_6Gen_2022428_600M", \
                "Raw/raw_seeds_TCP6_ACK_6Scan_2022428_1000M", "Raw/raw_seeds_TCP6_ACK_6Scan_2022428_1500M",]
    file_list = ["Raw/raw_seeds_TCP6_ACK_6Scan_2022428_1500M", "Raw/raw_seeds_TCP6_ACK_6Gen_2022428_1500M"]         
    
    seed_file = seeds(probe)
    seed_set = set()
    with open(seed_file) as f:
        temp_list = f.read().splitlines()
        seed_set = set(temp_list) 
        print("Seeds number:", len(seed_set))       
    f.close()

    discovery_last = 0
    budget_last = 0
    
    for file_name in file_list:
        IP_set = set()
        IP_list = list()
        print("--------------------")
        print(file_name)
        with open(file_name) as f:
            temp_list = f.read().splitlines()
            for temp in temp_list:
                if temp[0] != '#':
                    index = temp.find(',')
                    IP_list.append(temp[:index])
        f.close()
        IP_set = set(IP_list)
        print("Address number:", len(IP_set))
        discovery = len(IP_set - seed_set)
        print("Discovery number:", discovery)
        index = file_name.rfind('_')
        budget = int(file_name[index + 1 : -1]) * 1000000
        print("Discovery rate:", discovery / budget)
        # print("Marginal benefit:", (discovery - discovery_last) / (budget - budget_last))
        # discovery_last = discovery
        # budget_last = budget
        