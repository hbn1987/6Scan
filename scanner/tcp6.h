/****************************************************************************
 Copyright (c) 2021-2022 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
class TCP6 {
    public:
    TCP6(struct ip6_hdr *ip, struct tcphdr *tcpheader);
    void print();
    void write(FILE ** out, Stats* stats, bool probe_type);
    void write2seeds(FILE ** out, Stats* stats, bool probe_type);
    bool is_scan;
    uint8_t flags;
    std::string type_str;
    struct scanpayload *qpayload = NULL;  
    struct in6_addr ip_src;
    struct in6_addr *scan_target;
    uint32_t fingerprint;
};