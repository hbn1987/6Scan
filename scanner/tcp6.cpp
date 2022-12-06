/****************************************************************************
 Copyright (c) 2021-2022 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/

#include "6scan.h"

TCP6::TCP6(struct ip6_hdr *ip, struct tcphdr *tcpheader):
   flags(0), fingerprint(0)
{
    /* Ethernet
     *  IPv6 hdr                struct ip6_hdr *ip;
     *  Probe transport hdr     struct tcphdr *tcpheader;
     *  6scan payload           struct scanpayload *qpayload;
     */

    memset(&ip_src, 0, sizeof(struct in6_addr));    
    ip_src = ip->ip6_src; 
    is_scan = false;
    flags = tcpheader->th_flags;
    type_str = "TCP6_";

    if (flags & TH_FIN)
        type_str += "FIN_";
    if (flags & TH_SYN)
        type_str += "SYN_"; 
    if (flags & TH_RST)
        type_str += "RST_";
    if (flags & TH_PUSH)
        type_str += "PUSH_";
    if (flags & TH_ACK)
        type_str += "ACK_";
    if (flags & TH_URG)
        type_str += "URG_";

    unsigned char *ptr = (unsigned char *) tcpheader;
    qpayload = (struct scanpayload *) (ptr + sizeof(struct tcphdr));
    if (ntohl(qpayload->id) == 0x06536361)
        is_scan = true;
    scan_target = &(qpayload->target); 
}

void TCP6::print() {
    char src[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip_src, src, INET6_ADDRSTRLEN);    
    printf("TCP6 response from: %s type: %s\n", src, type_str.c_str());
}

void TCP6::write(FILE ** out, Stats* stats, bool probe_type) {
    if (*out == NULL)
        return;
    char src[INET6_ADDRSTRLEN];
    char target[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip_src, src, INET6_ADDRSTRLEN);  

    if (is_scan) {
        type_str += "withPayload_";
        inet_ntop(AF_INET6, scan_target, target, INET6_ADDRSTRLEN);
        if (strcmp(src, target) == 0) {   
            type_str += "Target";   
            if ((stats->strategy == Scan6) or (stats->strategy == Hit6)) {        
                uint64_t index = ntohl(qpayload->fingerprint);
                if (index < stats->nodelist.size())
                    stats->nodelist[index]->active++;
                else {
                    warn("Returning error regional identification %lu", index);
                    stats->baddst++;
                }
            } else if (stats->strategy == Heuristic) {               
                string addr = seed2vec(src);            
                string prefix = addr.substr(0, stats->mask/4);
                unordered_map<string, int>::iterator iter = stats->prefix_map.find(prefix);
                if (iter != stats->prefix_map.end())
                    iter->second++;
                if (addr.substr(addr.length()-4) != "1234") { // If the address is not the pseudorandom address, write it into the hitlist
                    stats->prefixes.push_back(addr);
                    fprintf(*out, "%s\n", src);
                }
            }
        } else
            type_str += "Src";
    } else 
        type_str += "noPayload";

    if (probe_type)
        fprintf(*out, "%s, %s, %s\n", src, type_str.c_str(), target);
    else
        fprintf(*out, "%s\n", src);
}