/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
struct scanpayload {
    uint32_t id;
    uint32_t fingerprint;
    struct in6_addr target; /* v6 target address, to detect spoofed responses */
    uint8_t instance; /* instance */
    uint8_t ttl;      /* sent TTL */
    uint16_t fudge;   /* make chksum constant */
    uint32_t diff;    /* elapsed time */
};

class Traceroute {
    public:
    Traceroute(ScanConfig *config, Stats *stats);
    virtual ~Traceroute();
    void addStats(Stats *_stats) {
        stats = _stats;
    }
    uint32_t elapsed();
    void lock();
    void unlock();
    virtual void probe(uint32_t, int) {};
    virtual void probe(struct sockaddr_in *, int) {};
    virtual void probePrint(struct in_addr *, int) {};
    virtual void probe(struct in6_addr, int) {};
    virtual void probePrint(struct in6_addr, int) {};

    public:
    Stats *stats;
    ScanConfig *config;

    protected:
    int sndsock; /* raw socket descriptor */
    int payloadlen;
    int packlen;
    pthread_t recv_thread;
    pthread_mutex_t recv_lock;
    uint16_t dstport;
    struct timeval start;
    struct timeval now;
};

class Traceroute4 : public Traceroute {
    public:
    Traceroute4(ScanConfig *config, Stats *stats);
    virtual ~Traceroute4();
    struct sockaddr_in *getSource() { return &source; }
    void probe(const char *, int);
    void probe(uint32_t, int);
    void probe(struct sockaddr_in *, int);
    void probePrint(struct in_addr *, int);

    private:
    void probeUDP(struct sockaddr_in *, int);
    void probeTCP(struct sockaddr_in *, int);
    void probeICMP(struct sockaddr_in *, int);
    struct ip *outip;
    struct sockaddr_in source;
    char addrstr[INET_ADDRSTRLEN];
};

class Traceroute6 : public Traceroute {
    public:
    Traceroute6(ScanConfig *config, Stats *stats);
    virtual ~Traceroute6();
    struct sockaddr_in6 *getSource() { return &source6; }
    void probe(struct in6_addr, int);
    void probePrint(struct in6_addr, int);
    void probe(void *, struct in6_addr, int);
    void change_fingerprint(uint64_t);
    struct sockaddr_in6 source6;

    private:
    void make_transport(int);
    void make_frag_eh(uint8_t);
    void make_hbh_eh(uint8_t);
    struct ip6_hdr *outip;
    uint8_t *frame;
    int pcount;
    uint8_t tc = 0; /* traffic class which we always set to 0 */
    uint32_t flow = 0; /* flow label which we always set to 0 */
    struct scanpayload *payload;
    char addrstr[INET6_ADDRSTRLEN];
};
