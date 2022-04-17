/****************************************************************************
 Copyright (c) 2021-2022 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/

#include "6scan.h"

UDP6::UDP6(struct ip6_hdr *ip, struct udphdr *udpheader):
   flags(0), fingerprint(0)
{
    /* Ethernet
     *  IPv6 hdr                struct ip6_hdr *ip;
     *  Probe transport hdr     struct udphdr *udpheader;
     *  6scan payload           struct scanpayload *qpayload;
     */

    memset(&ip_src, 0, sizeof(struct in6_addr));    
    ip_src = ip->ip6_src;  

    is_scan = false;
    type_str = "UDP6";

    unsigned char *ptr = (unsigned char *) udpheader;
    qpayload = (struct scanpayload *) (ptr + sizeof(struct udphdr));
    if (ntohl(qpayload->id) == 0x06536361) {
        is_scan = true;
        type_str += "_withPayload";
    }
    else
        type_str += "_noPayload";  

    scan_target = &(qpayload->target); 
}

void UDP6::print() {
    char src[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip_src, src, INET6_ADDRSTRLEN);    
    printf("UDP6 response from: %s type: %s\n", src, type_str.c_str());
}

void UDP6::write(FILE ** out, Stats* stats, bool probe_type) {
    if (*out == NULL)
        return;
    char src[INET6_ADDRSTRLEN];
    char target[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip_src, src, INET6_ADDRSTRLEN);    
    
    if (probe_type)
        fprintf(*out, "%s, %s\n", src, type_str.c_str());
    else
        fprintf(*out, "%s\n", src);

    if (is_scan) {
        switch (stats->strategy) {        
        case Scan6:
        case Hit6:
            uint64_t index = ntohl(qpayload->fingerprint);
            if (index < stats->nodelist.size())
                stats->nodelist[index]->active++;
            else {
                warn("Returning error regional identification %lu", index);
                stats->baddst++;
            }
            break;
        }        
    }
}

void UDP6::write2seeds(FILE ** out, Stats* stats, bool probe_type) { // Pre-scan and Heuristic use this function
    char src[INET6_ADDRSTRLEN];
    char target[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip_src, src, INET6_ADDRSTRLEN);
    if (is_scan)
        inet_ntop(AF_INET6, scan_target, target, INET6_ADDRSTRLEN);    
    else
        inet_ntop(AF_INET6, &ip_src, target, INET6_ADDRSTRLEN);

    if (strcmp(src, target) == 0) {      
        if (stats->strategy == Heuristic) {                        
            string addr = seed2vec(src);            
            string prefix = addr.substr(0, stats->mask/4);
            unordered_map<string, int>::iterator iter = stats->prefix_map.find(prefix);
            if (iter != stats->prefix_map.end())
                iter->second++;
            if (addr.substr(addr.length()-4) != "1234") { // If the address is not the pseudorandom address, write it into the hitlist
                stats->prefixes.push_back(addr);
                fprintf(*out, "%s\n", src);
            }
        } else { // Pre-scan
            if (probe_type)
                fprintf(*out, "%s, %s\n", src, type_str.c_str());
            else
                fprintf(*out, "%s\n", src);
        }
    }
}