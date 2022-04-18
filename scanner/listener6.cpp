/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
#include "6scan.h"
#include <signal.h>

static volatile bool run = true;
void intHandler(int dummy);
void *listener6(void *args) {
    fd_set rfds;
    Traceroute6 *trace = reinterpret_cast < Traceroute6 * >(args);
    struct timeval timeout;
    unsigned char *buf = (unsigned char *) calloc(1,PKTSIZE);
    uint32_t nullreads = 0;
    int n, len;
    uint32_t elapsed = 0;
    struct ip6_hdr *ip = NULL;                /* IPv6 hdr */
    struct icmp6_hdr *ippayload = NULL;       /* ICMP6 hdr */
    struct tcphdr *tcpheader = NULL;
    struct udphdr *udpheader = NULL;
    int rcvsock;                              /* receive (icmp) socket file descriptor */
    struct scanpayload *payload = NULL;

    /* block until main thread says we're ready. */
    trace->lock();
    trace->unlock();

    if ((rcvsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        cerr << "6scan listener socket error:" << strerror(errno) << endl;
    }

    signal(SIGINT, intHandler);
    while (true and run) {
        if (nullreads >= MAXNULLREADS)
            break;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(rcvsock, &rfds);
        n = select(rcvsock + 1, &rfds, NULL, NULL, &timeout);
        if (n == 0) {
            nullreads++;
            cerr << "Listener: timeout " << nullreads;
            cerr << "/" << MAXNULLREADS << endl;
            continue;
        }
	    if (n == -1) {
            fatal("select error");
        }
        nullreads = 0;
        len = recv(rcvsock, buf, PKTSIZE, 0);

        if (len == -1) {
            fatal("%s %s", __func__, strerror(errno));
        }
        ip = (struct ip6_hdr *)(buf + ETH_HDRLEN);

        if (ip->ip6_nxt == IPPROTO_ICMPV6) {
            ippayload = (struct icmp6_hdr *)&buf[ETH_HDRLEN + sizeof(struct ip6_hdr)];
            elapsed = trace->elapsed();
            if ((ippayload->icmp6_type == ICMP6_TIME_EXCEEDED) or
                 (ippayload->icmp6_type == ICMP6_DST_UNREACH) or
                 (ippayload->icmp6_type == ICMP6_ECHO_REPLY)) {
                ICMP6 *icmp = new ICMP6(ip, ippayload, elapsed);
                // icmp->print();
                icmp->write(&(trace->config->out), trace->stats, trace->config->probe_type);
                delete icmp;
            }
        } else if (ip->ip6_nxt == IPPROTO_UDP) {
            char src[INET6_ADDRSTRLEN], hostaddr[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &ip->ip6_src, src, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &trace->source6.sin6_addr, hostaddr, INET6_ADDRSTRLEN);
            if (strcmp(src, hostaddr) != 0 ) {
                udpheader = (struct udphdr *)&buf[ETH_HDRLEN + sizeof(struct ip6_hdr)];
                UDP6 *udp = new UDP6(ip, udpheader);
                // udp->print();
                udp->write(&(trace->config->out), trace->stats, trace->config->probe_type);
                delete udp;
            }
        } else if (ip->ip6_nxt == IPPROTO_TCP) {
            char src[INET6_ADDRSTRLEN], hostaddr[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &ip->ip6_src, src, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &trace->source6.sin6_addr, hostaddr, INET6_ADDRSTRLEN);
            if (strcmp(src, hostaddr) != 0 ) {
                tcpheader = (struct tcphdr *)&buf[ETH_HDRLEN + sizeof(struct ip6_hdr)];
                TCP6 *tcp = new TCP6(ip, tcpheader);
                // tcp->print();
                tcp->write(&(trace->config->out), trace->stats, trace->config->probe_type);
                delete tcp;
            }
        }
    }
    return NULL;
}
