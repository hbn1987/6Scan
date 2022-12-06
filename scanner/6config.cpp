/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "6scan.h"

static struct option long_options[] = {
    {"srcaddr", required_argument, NULL, 'a'},
    {"alias", required_argument, NULL, 'A'},
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

    params["Program"] = val_t("6Scan", true);

    while (-1 != (c = getopt_long(argc, argv, "a:b:A:C:D:dE:F:G:hHI:l:L:M:p:Pr:s:t:X:", long_options, &opt_index))) {
        switch (c) {
        case 'a':
            probesrc = optarg;
            break;
        case 'A':
            alias = true;
            alias_range = optarg;            
            strategy = Heuristic;
            break;
        case 'b':
            budget = strtol(optarg, &endptr, 10) * 1000000;
            break;
        case 'C':
            classification = optarg;
            break;
        case 'd':
            probe_type = true;
            break;
        case 'D':
            download = optarg;
            break;
        case 'F':
            seedfile = optarg;
            break;
        case 'G':
            dstmac = read_mac(optarg);
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 'H':
            hitlist = true;
            break;
        case 'I':
            int_name = optarg;
            break;
        case 'l':
            region_limit = optarg;
            break;
        case 'L':
            level = optarg;
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
            rate = strtol(optarg, &endptr, 10) * 1000 * 1.25;
            break;
        case 's':
            if (strcmp(optarg, "6Scan") == 0) {
                strategy = Scan6;
            } else if(strcmp(optarg, "6Hit") == 0) {
                strategy = Hit6;
            } else if(strcmp(optarg, "6Tree") == 0) {
                strategy = Tree6;
            } else if(strcmp(optarg, "6Gen") == 0) {
                strategy = Gen6;
            } else if (strcmp(optarg, "HMap6") == 0) {
                strategy = HMap6;
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

    if (hitlist) {
        if (region_limit) {
            string type_str = Tr_Type_String[type];
            string seedset = get_seedset(type_str);
            hitlist_region_seeds(seedset, string(region_limit), type_str);
        } else if (strcmp(level, "alias") != 0) {
            string type_str = Tr_Type_String[type];
            string seedset = get_seedset(type_str);
            hitlist_analysis(seedset, string(level));
        } else {
            string alias_file = get_aliasfile();
            alias_analysis(alias_file);
        }
        exit(-1);
    }

    /* Set default output file */
    if (0 != access(string(OUTPUT).c_str(),0))
        mkdir(string(OUTPUT).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    output = (char *) malloc (UINT8_MAX);

    string scan_time = get_scan_time();

    if (pre_scan) {
        string seed_file = "./output/seeds_" + string(Tr_Type_String[type]) + "_" + scan_time;
        if (probe_type)
            seed_file += "_probetype";
        snprintf(output, UINT8_MAX, "%s", seed_file.c_str());
    }

    if (alias) {
        string hitlist_file = "./output/hitlist_" + string(alias_range) + "_" + string(Tr_Type_String[type]) + "_" + scan_time;
        string alias_file = "./output/alias_" + string(alias_range) + "_"+ string(Tr_Type_String[type]) + "_" + scan_time;
        snprintf(output, UINT8_MAX, "%s", hitlist_file.c_str());

        alias_output = (char *) malloc (UINT8_MAX);
        snprintf(alias_output, UINT8_MAX, "%s", alias_file.c_str());
        alias_out = fopen(alias_output, "w+");
        if (alias_out == NULL)
            fatal("%s: cannot open %s: %s", __func__, alias_output, strerror(errno));
    }

    if (strategy and not alias) {
        string result_file;
        if (region_limit)
            result_file = "./output/raw_" + string(region_limit) + "_" + string(search_strategy_str[strategy]) + "_" + string(Tr_Type_String[type]) + "_" + scan_time;
        else if (seedfile) {
            int index = string(seedfile).find_last_of('_');
            result_file = "./output/raw_" + string(seedfile).substr(9, index - 8) + string(search_strategy_str[strategy]) + "_" + scan_time;
        } else if (probe_type)
            result_file = "./output/raw_" + string(search_strategy_str[strategy]) + "_" + string(Tr_Type_String[type]) + "_" + scan_time + "_probetype";
        else
            result_file = "./output/raw_" + string(search_strategy_str[strategy]) + "_" + string(Tr_Type_String[type]) + "_" + scan_time;
        
        snprintf(output, UINT8_MAX, "%s", result_file.c_str());

        if (strategy == HMap6) {
            string space_file = "./output/subspace_" + string(search_strategy_str[strategy]) + "_" + string(Tr_Type_String[type]) + "_" + scan_time;
            space_output = (char*) malloc(strlen(space_file.c_str()) + 1);
            strcpy(space_output, space_file.c_str());
            space_out = fopen(space_output, "w+");
            if (space_out == NULL)
                fatal("%s: cannot open %s: %s", __func__, space_output, strerror(errno));
        }
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
        if (string(classification).find("country") != string::npos) {
            auto pos = string(classification).find("country");
            region_limit = (char*) malloc (10 * sizeof(char));
            strncpy(region_limit, classification + pos, 10);
            region_limit[10] = '\0';       
        }
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

    if (not budget) {
        budget = BUDGET;
    }
    dimension = int(log(budget)/log(16));

    std::cout << "Start running 6Scan..." << endl;
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
    std::cout << "Usage: " << prog << " [OPTIONS] [targets]" << endl
    << "  -t, --type              Probe type: ICMP6, UDP6, TCP6_SYN, TCP6_ACK" << endl 
    << "  -r, --rate              Scan rate in pps (default: 10)" << endl
    << "  -a, --srcaddr           Source address of probes (default: auto)" << endl
    << "  -p, --port              Transport dst port (default: 80)" << endl
    << "  -I, --interface         Network interface (required for IPv6)" << endl
    << "  -G, --dstmac            MAC of gateway router (default: auto)" << endl
    << "  -M, --srcmac            MAC of probing host (default: auto)" << endl
    << "  -X, --v6eh              Ext Header number to add (default: none)" << endl
    << "  See README for more details!" << endl;
    exit(-1);
}