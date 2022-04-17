/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
typedef std::pair<std::string, bool> val_t;
typedef std::map<std::string, val_t> params_t;

class ScanConfig {
public:
    ScanConfig() : rate(0), output(NULL), count(0), seed(0),
    dstport(80), ipv6(false), int_name(NULL), dstmac(NULL),
    srcmac(NULL), probesrc(NULL), instance(0), v6_eh(255),
    out(NULL), pre_scan(false), alias(false), type(0), 
    strategy(0), classification(NULL), download(NULL), alias_range(NULL),
    alias_output(NULL), alias_out(NULL), region_limit(NULL), hitlist(false),
    level(NULL), dealias(NULL), seedfile(NULL), probe_type(false){};

    void parse_opts(int argc, char **argv);
    void usage(char *prog);
    void set(std::string, std::string, bool);
    unsigned int rate;
    char *output;
    char *alias_output;
    uint64_t count;
    uint32_t seed;
    uint16_t dstport;
    bool ipv6;
    char *int_name;
    uint8_t *dstmac;
    uint8_t *srcmac;
    int strategy;
    int type;
    char *probesrc;
    uint8_t instance;
    uint8_t v6_eh;
    FILE *out;
    FILE *alias_out;
    params_t params;
    bool pre_scan;
    bool alias;
    bool hitlist;
    char* download;
    char* alias_range;
    char* classification;
    char* region_limit;
    char* level;
    char* dealias;
    char* seedfile;
    bool probe_type;
};