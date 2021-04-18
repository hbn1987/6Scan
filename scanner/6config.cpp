#include <ctime>
#include "6scan.h"

static struct option long_options[] = {
    {"srcaddr", required_argument, NULL, 'a'},
    {"classification", required_argument, NULL, 'C'},
    {"download", required_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {"interface", required_argument, NULL, 'I'},
    {"dstmac", required_argument, NULL, 'G'},
    {"srcmac", required_argument, NULL, 'M'},
    {"port", required_argument, NULL, 'p'},
    {"pre-scan", no_argument, NULL, 'P'},
    {"rate", required_argument, NULL, 'r'},
    {"strategy", required_argument, NULL, 's'},
    {"type", required_argument, NULL, 't'},
    {"v6eh", no_argument, NULL, 'X'},
    {NULL, 0, NULL, 0},
};

uint8_t *read_mac(char *str) {
    uint8_t *mac = (uint8_t *) malloc (6 * sizeof(uint8_t));
    mac[0] = (uint8_t) strtoul(strtok(str, ":"), NULL, 16);
    for (int i=1; i < 6; i++) 
        mac[i] = (uint8_t) strtoul(strtok(NULL, ":"), NULL, 16);
    return mac;
}

void ScanConfig::parse_opts(int argc, char **argv) {
    int c, opt_index;
    char *endptr;

    if (argc <= 1)
        usage(argv[0]);
    seed = time(NULL);

    params["Program"] = val_t("6Scan v" + string(VERSION), true);

    while (-1 != (c = getopt_long(argc, argv, "a:C:D:E:G:hI:M:p:Pr:s:t:X:", long_options, &opt_index))) {
        switch (c) {
        case 'a':
            probesrc = optarg;
            break;
        case 'C':
            classification = optarg;
            break;
        case 'D':
            download = optarg;
            break;
        case 'G':
            dstmac = read_mac(optarg);
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 'I':
            int_name = optarg;
            break;
        case 'M':
            srcmac = read_mac(optarg);
            break;
        case 'p':
            dstport = strtol(optarg, &endptr, 10);
            break;
        case 'P':
            pre_scan = true;
            break;
        case 'r':
            rate = strtol(optarg, &endptr, 10);
            break;
        case 's':
            if (strcmp(optarg, "6Scan") == 0) {
                strategy = Scan6;
            } else if(strcmp(optarg, "6Tree") == 0) {
                strategy = Tree6;
            } else if(strcmp(optarg, "6Gen") == 0) {
                strategy = Gen6;
            } else if(strcmp(optarg, "Edgy") == 0) {
                strategy = Edgy;
            }
            break;
        case 't':
            if (strcmp(optarg, "ICMP6") == 0) {
                ipv6 = true;
                type = TR_ICMP6;
            } else if(strcmp(optarg, "UDP6") == 0) {
                ipv6 = true;
                type = TR_UDP6;
            } else if(strcmp(optarg, "TCP6_SYN") == 0) {
                ipv6 = true;
                type = TR_TCP6_SYN;
            } else if(strcmp(optarg, "TCP6_ACK") == 0) {
                ipv6 = true;
                type = TR_TCP6_ACK;
            } else if(strcmp(optarg, "ICMP") == 0) {
                type = TR_ICMP;
            } else if(strcmp(optarg, "ICMP_REPLY") == 0) {
                type = TR_ICMP_REPLY;
            } else if(strcmp(optarg, "UDP") == 0) {
                type = TR_UDP;
            } else if(strcmp(optarg, "TCP_SYN") == 0) {
                type = TR_TCP_SYN;
            } else if(strcmp(optarg, "TCP_ACK") == 0) {
                type = TR_TCP_ACK;
            } else {
                usage(argv[0]);
            }
            break;
        case 'X':
            v6_eh = strtol(optarg, &endptr, 10);
            break;
        default:
            usage(argv[0]);
        }
    }

    if (download) {
        data_download(string(download));
        exit(-1);
    }
    /* Set default output file */
    if (0 != access(string(OUTPUT).c_str(),0))
        mkdir(string(OUTPUT).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    output = (char *) malloc(UINT8_MAX);

    string scan_time = get_scan_time();

    if (pre_scan) {
        string seed_file = "./output/seeds_" + string(Tr_Type_String[type]) + "_" + scan_time;
        snprintf(output, UINT8_MAX, "%s" , seed_file.c_str());
    }

    if (strategy) {
        string result_file = "./output/raw_" + string(search_strategy_str[strategy]) + "_" + string(Tr_Type_String[type]) + "_" + scan_time;
        snprintf(output, UINT8_MAX, "%s", result_file.c_str());
    }

    if (classification) {
        string result_file = "./output/results_" + string(classification).substr(13);
        snprintf(output, UINT8_MAX, "%s", result_file.c_str());
        if (string(classification).find("ICMP6") != string::npos)
            type = TR_ICMP;
        else if (string(classification).find("TCP6") != string::npos)
            type = TR_TCP_SYN;
        else if(string(classification).find("UDP6") != string::npos)
            type = TR_UDP;
    }

    out = fopen(output, "w+");
    if (out == NULL)
        fatal("%s: cannot open %s: %s", __func__, output, strerror(errno));

    /* Set default destination port based on tracetype, if not set */
    if (not dstport) {
        dstport = 80;
        if ( (type == TR_UDP) || (type == TR_UDP6) )
            dstport = 53;
    }
    // Set probing rate, if not set
    if (not rate)
        rate = RATE;

    cout << "Start running 6Scan..." << endl;
    params["Seed"] = val_t(to_string(seed), true);
    params["Rate"] = val_t(to_string(rate), true);
    params["Trace_Type"] = val_t(Tr_Type_String[type], true);
    if (ipv6) {
        params["v6_EH"] = val_t(to_string(v6_eh), true);
    }
    params["Start"] = val_t("unknown", true);
    params["Dst_Port"] = val_t(to_string(dstport), true);
    params["Output_Fields"] = val_t("target sec usec type code ttl hop rtt ipid psize rsize rttl rtos mpls count", true);
}


void ScanConfig::set(string key, string val, bool isset) {
    params[key] = val_t(val, isset);
}

void ScanConfig::usage(char *prog) {
    cout << "Usage: " << prog << " [OPTIONS] [targets]" << endl
    << "  -t, --type              Probe type: ICMP, ICMP_REPLY, TCP_SYN, TCP_ACK, UDP," << endl
    << "                                      ICMP6, UDP6, TCP6_SYN, TCP6_ACK" << endl 
    << "                                      (default: TCP_ACK)" << endl
    << "  -r, --rate              Scan rate in pps (default: 10)" << endl
    << "  -a, --srcaddr           Source address of probes (default: auto)" << endl
    << "  -C  --classification    Classification the active addresses" << endl
    << "  -p, --port              Transport dst port (default: 80)" << endl
    << "  -I, --interface         Network interface (required for IPv6)" << endl
    << "  -G, --dstmac            MAC of gateway router (default: auto)" << endl
    << "  -M, --srcmac            MAC of probing host (default: auto)" << endl
    << "  -X, --v6eh              Ext Header number to add (default: none)" << endl

    << endl;
    exit(-1);
}
