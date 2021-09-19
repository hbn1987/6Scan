#if HAVE_CONFIG_H
    #include "config.h"
#endif

#ifndef _6SCAN_H_
#define _6SCAN_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#ifndef __FAVOR_BSD
    #define  __FAVOR_BSD
#endif
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#include <net/if.h>
#ifdef HAVE_NETINET_IP_VAR_H
    #include <netinet/ip_var.h>
#endif
#ifdef HAVE_NETINET_UDP_VAR_H
    #include <netinet/udp_var.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
    #include <stdint.h>
#endif
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <cmath>
#ifdef HAVE_PTHREAD
    #include <pthread.h>
#endif
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <bitset>
#include <utility>
#include <cstring>
#include <ctime>

#include "libcperm/cperm.h"

typedef enum {TR_ICMP6 = 1, TR_ICMP, TR_UDP6, TR_UDP,
              TR_TCP6_SYN, TR_TCP_SYN, TR_TCP6_ACK,
              TR_TCP_ACK, TR_ICMP_REPLY} traceroute_type;

static const char *Tr_Type_String[] = {"", "ICMP6", "ICMP", "UDP6", "UDP",
                                       "TCP6_SYN", "TCP_SYN", "TCP6_ACK",
                                       "TCP_ACK", "ICMP_REPLY"};

typedef enum {Scan6 = 1, Hit6, Tree6, Gen6, Heuristic} search_strategy;

static const char *search_strategy_str[] = {"", "6Scan", "6Hit", "6Tree", "6Gen", "Heuristic"};

#define warn(x...) do {fprintf(stderr,"*** Warn: "); fprintf(stderr,x); fprintf(stderr,"\n");} while (0)
#define fatal(x...) do {fprintf(stderr,"*** Fatal: "); fprintf(stderr,x); fprintf(stderr,"\n"); exit(-1);} while (0)

#define PKTSIZE 1500
#define MAXNULLREADS 10
#define SHUTDOWN_WAIT 20
#define KEYLEN 16
#ifndef UINT8_MAX
    #define UINT8_MAX (255)
    #define UINT16_MAX (65535)
    #define UINT32_MAX (4294967295U)
#endif
#define ETH_HDRLEN 14

/* IPv6 address manipulation */
#ifndef s6_addr16
#define s6_addr16	__u6_addr.__u6_addr16
#endif
#ifndef s6_addr32
#define s6_addr32	__u6_addr.__u6_addr32
#endif

unsigned short in_cksum(unsigned short *addr, int len);
int infer_my_ip(struct sockaddr_in *mei);
int infer_my_ip6(struct sockaddr_in6 *mei6);
int raw_sock(struct sockaddr_in *sin_orig);
int raw_sock6(struct sockaddr_in6 *sin6_orig);
u_short p_cksum(struct ip *ip, u_short *data, int len);
u_short p_cksum(struct ip6_hdr *ip, u_short *data, int len);
unsigned short compute_data(unsigned short start_cksum, unsigned short target_cksum);
void print_binary(const unsigned char *buf, int len, int brk, int tabs);
void *listener(void *args);
void *listener6(void *args);
uint32_t tsdiff(struct timeval *end, struct timeval *begin);
uint32_t tsdiffus(struct timeval *end, struct timeval *begin);
double now(void);
uint8_t randuint8();
bool checkRoot();
void instanceLock(uint8_t);
double zrand();
void permseed(uint8_t *);
void permseed(uint8_t *, uint32_t);
uint32_t intlog(uint32_t in);
std::string get_alias(std::string line, int mask);

#include "6config.h"
#include "mac.h"
#include "iplist.h"
#include "stats.h"
#include "trace.h"
#include "icmp.h"

/* 6Scan parameters */
#define DOWNLOAD "./download" // Hitlist and alias prefixes
#define OUTPUT "./output" // Output folder
#define RATE 100000 // Probing rate
#define TTL 128
#define BUDGET 10000000 // Budget number

#define Alias_Threshold 0.9
#define DIMENSION int(log(BUDGET)/log(16))

#include "strategy/strategy.h"

using namespace std;

#endif  /* _6SCAN_H_ */