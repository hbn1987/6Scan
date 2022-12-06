import pyasn
asndb = pyasn.pyasn('./analysis/data/ipasn_20221106.dat')
def AS_consistency(ipdict):
    for target, src in ipdict.items():
        asn_tar, prefix = asndb.lookup(target)
        asn_src, prefix = asndb.lookup(src) 
        if asn_tar != asn_src:
            print('Target:', target, 'Target ASN:', asn_tar)
            print('Src:', src, 'Src ASN:', asn_src)

def Extract_nonTarget(file_name):
    target_src = dict()
    with open(file_name) as f:
        IPs_types = f.read().splitlines()
        for ip_type in IPs_types:
            if ip_type[0] == '#':
                continue
            if (ip_type.find('Src') != -1):
                index1 = ip_type.find(',')
                src = ip_type[0:index1]     
                index2 = ip_type.find(',', index1+1)
                target = ip_type[index2+2:]  
                target_src[target] = src
    print("Non-Target reply:", len(target_src))
    print("Unique non-target (src) address number:", len(set(target_src.values())))
    AS_consistency(target_src)
    # num = 0
    # for k, v in target_src.items():
    #     num += 1
    #     if num == 10:
    #         break
    #     print('target:', k, 'src:', v)



if __name__ == "__main__":
    file_list = ["output/seeds_ICMP6_20221112_probetype"]
    for i in range(len(file_list)):
        Extract_nonTarget(file_list[i])
