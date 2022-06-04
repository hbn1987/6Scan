def Extract_nonTarget(file_name):
    src = set()
    with open(file_name) as f:
        IPs_types = f.read().splitlines()
        for ip_type in IPs_types:
            if ip_type[0] == '#':
                continue
            if (ip_type.find('Src') != -1):
                index = ip_type.find(',')
                ip = ip_type[0:index]     
                src.add(ip)
    print("Non-target address number:", len(src))
    
    # new_file_name = file_name.replace('Raw', 'output')
    # new_file_name = new_file_name.replace('1500M', 'non-target')
    # with open(new_file_name, 'w') as f:
    #     for ip in src:
    #         f.write("%s\n" % ip)

if __name__ == "__main__":
    file_list = ["Raw/raw_seeds_ICMP6_6Scan_2022424_1500M", "Raw/raw_seeds_UDP6_6Scan_2022426_1500M", "Raw/raw_seeds_TCP6_ACK_6Gen_2022428_1500M"]
    for i in range(len(file_list)):
        Extract_nonTarget(file_list[i])
