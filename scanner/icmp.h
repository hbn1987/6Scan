/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
typedef struct mpls_label {
    uint32_t label:20;
    uint8_t exp:4;
    uint8_t ttl;
    struct mpls_label *next;
} mpls_label_t;
#define MAX_MPLS_STACK_HEIGHT 4

class ICMP {
    public:
    ICMP();
    virtual void print() {};
    virtual void write(FILE **, Stats *) {};
    virtual void write2seeds(FILE **, Stats *) {};
    virtual uint32_t getSrc() { return 0; };
    virtual struct in6_addr *getSrc6() { return NULL; };
    virtual uint32_t quoteDst() { return 0; };
    virtual struct in6_addr quoteDst6() { struct in6_addr a; return a; };
    void printterse(char *);
    uint8_t quoteTTL() { return ttl; }
    uint32_t getRTT() { return rtt; }
    uint32_t getTTL() { return ttl; }
    uint16_t getSport() { return sport; }
    uint16_t getDport() { return dport; }
    uint8_t  getInstance() { return instance; }
    void print(char *, char *, int);
    void write(FILE **, char *, char *);
    char *getMPLS();
    bool is_scan;

    protected:
    uint32_t rtt;
    uint8_t ttl;
    uint8_t instance;
    uint8_t type;
    uint8_t code;
    uint8_t length;
    uint8_t quote_p;
    uint16_t sport;
    uint16_t dport;
    uint16_t ipid;
    uint16_t probesize;
    uint16_t replysize;
    uint8_t replyttl;
    uint8_t replytos;
    struct timeval tv;
    mpls_label_t *mpls_stack;
    uint32_t fingerprint;
};

class ICMP4 : public ICMP {
    public:
    ICMP4(struct ip *, struct icmp *, uint32_t elapsed);
    uint32_t quoteDst();
    uint32_t getSrc() { return ip_src.s_addr; }
    void print();
    void write(FILE **, Stats *);
    void insert_ip_set(Stats *);

    private:
    struct ip *quote;
    struct in_addr ip_src;
};

class ICMP6 : public ICMP {
    public:
    ICMP6(struct ip6_hdr *, struct icmp6_hdr *, uint32_t elapsed);
    struct in6_addr *getSrc6() { return &ip_src; }
    struct in6_addr quoteDst6();
    void print();
    void write(FILE **, Stats *);
    void write2seeds(FILE **, Stats *);
    struct scanpayload *qpayload = NULL;     /* Quoted ICMPv6 payload */

    private:
    struct ip6_hdr *quote;
    struct in6_addr ip_src;
    struct in6_addr *scan_target;
};

