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
    unsigned char *ptr = (unsigned char *) udpheader;
    qpayload = (struct scanpayload *) (ptr + sizeof(struct udphdr));
    if (ntohl(qpayload->id) == 0x06536361)
        is_scan = true;
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
    type_str = "UDP6_";
    if (is_scan) {
        type_str += "withPayload_";
        inet_ntop(AF_INET6, scan_target, target, INET6_ADDRSTRLEN);
        if (stats->strategy == Heuristic) {               
            string addr = seed2vec(target);            
            string prefix = addr.substr(0, stats->mask/4);
            string hexString = addr.substr(stats->mask/4, 1);
            stats->hashMap.insert(prefix, hexString);
        }
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
            }
        } else
            type_str += "Src";
    } else 
        type_str += "noPayload";

    if (probe_type)
        fprintf(*out, "%s, %s, %s\n", target, src, type_str.c_str());
    else
        fprintf(*out, "%s\n", src);
}