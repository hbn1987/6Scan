typedef std::pair<std::string, bool> val_t;
typedef std::map<std::string, val_t> params_t;

class ScanConfig {
public:
    ScanConfig() : rate(0), output(NULL), count(0), seed(0),
    dstport(80), ipv6(false), int_name(NULL), dstmac(NULL),
    srcmac(NULL), probesrc(NULL), instance(0), v6_eh(255),
    out(NULL), pre_scan(false), type(0), strategy(0),
    classification(NULL), download(NULL){};

    void parse_opts(int argc, char **argv);
    void usage(char *prog);
    void set(std::string, std::string, bool);
    unsigned int rate;
    char *output;
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
    params_t params;
    bool pre_scan;
    char* download;
    char* classification;
};