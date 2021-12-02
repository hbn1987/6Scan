/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
#include "6scan.h"
#include <signal.h>

static volatile bool run = true;

void intHandler(int dummy) {
    run = false;
}

void *listener(void *args) {
    fd_set rfds;
    Traceroute *trace = reinterpret_cast < Traceroute * >(args);
    struct timeval timeout;
    unsigned char buf[PKTSIZE];
    uint32_t nullreads = 0;
    int n, len;
    uint32_t elapsed = 0;
    struct ip *ip = NULL;
    struct icmp *ippayload = NULL;
    int rcvsock; /* receive (icmp) socket file descriptor */

    /* block until main thread says we're ready. */
    trace->lock(); 
    trace->unlock(); 

    if ((rcvsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        cerr << "6scan listener socket error:" << strerror(errno) << endl;
    }

    while (true) {
        if (nullreads >= MAXNULLREADS)
            break;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(rcvsock, &rfds);
        n = select(rcvsock + 1, &rfds, NULL, NULL, &timeout);
        /* only timeout if we're also probing (not listen-only mode) */
        if (n == 0) {
            nullreads++;
            cerr << "Listener: timeout " << nullreads;
            cerr << "/" << MAXNULLREADS << endl;
            continue;
        }
        if (n > 0) {
            nullreads = 0;
            len = recv(rcvsock, buf, PKTSIZE, 0);
            if (len == -1) {
                cerr << "Listener: read error: " << strerror(errno) << endl;
                continue;
            }
            ip = (struct ip *)buf;
            if ((ip->ip_v == IPVERSION) and (ip->ip_p == IPPROTO_ICMP)) {
                ippayload = (struct icmp *)&buf[ip->ip_hl << 2];
                elapsed = trace->elapsed();
                ICMP4 *icmp = new ICMP4(ip, ippayload, elapsed);
                /* ICMP message not from 6scan instance, skip. */
                if (icmp->getInstance() != trace->config->instance) {
                    cout << "Listener: packet instance mismatch." << endl;
                    delete icmp;
                    continue;
                }
                //icmp->print();
                icmp->insert_ip_set(trace->stats);
                if (icmp->getSport() == 0)
                    trace->stats->baddst+=1;
                //icmp->write(&(trace->config->out), trace->stats);
                delete icmp;
            }
        }
    }
    return NULL;
}
