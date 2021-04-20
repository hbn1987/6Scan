#include "6scan.h"

template < class TYPE >
void loop(ScanConfig * config, TYPE * iplist, Traceroute * trace, Stats * stats) {
    struct in_addr target;
    struct in6_addr target6;
    uint8_t ttl = TTL;

    //adaptive timing to meet probing rate
    uint64_t count = 0;
    uint64_t last_count = count;
    uint64_t iteration_count = 0;
    double last_time = now();
    uint32_t delay = 0;
    int interval = 0;
    volatile int vi;
    struct timespec ts, rem;
    double send_rate = (double)config->rate;
    const double slow_rate = 50;
    long nsec_per_sec = 1000 * 1000 * 1000;
    long long sleep_time = nsec_per_sec;

    if (config->rate > 0) {
        delay = 10000;
        if (send_rate < slow_rate) {
            //set the inital time difference
            sleep_time = nsec_per_sec / send_rate;
            last_time = now() - (1.0 / send_rate);
        } else {
            //estimate initial rate
            for (vi = delay; vi--;);
            delay *= 1 / (now() - last_time) / (config->rate);
            interval = (config->rate) / 20;
            last_time = now();
        }
    }
    while (true) {
        /* Grab next target/ttl pair from permutation */
        if (config->ipv6) {
            if ((iplist->next_address(&target6)) == 0)
                break;
        } else {
            if ((iplist->next_address(&target)) == 0)
                break;
        }

        if (config->ipv6)
            trace->probe(target6, ttl);
        else
            trace->probe(target.s_addr, ttl);

        stats->count++;
        iteration_count++;

        /* Calculate sleep time based on scan rate */
        if (config->rate) {
            send_rate = (double)config->rate;
            if (count && delay > 0) {
                if (send_rate < slow_rate) {
                    double t = now();
                    double last_rate = (1.0 / (t - last_time));

                    sleep_time *= ((last_rate / send_rate) + 1) / 2;
                    ts.tv_sec = sleep_time / nsec_per_sec;
                    ts.tv_nsec = sleep_time % nsec_per_sec;
                    while (nanosleep(&ts, &rem) == -1) {
                    }
                    last_time = t;
                } else {
                    for (vi = delay; vi--;);
                    if (!interval || (count % interval == 0)) {
                        double t = now();
                        double multiplier = (double)(count - last_count) / (t - last_time) / (config->rate);
                        uint32_t old_delay = delay;
                        delay *= multiplier;
                        if (delay == old_delay) {
                            if (multiplier > 1.0) {
                                delay *= 2;
                            } else if (multiplier < 1.0) {
                                delay *= 0.5;
                            }
                        }
                        last_count = count;
                        last_time = t;
                    }
                }
            }
        }
        count = iteration_count;
        /* Quit if we've exceeded probe budget */
        if (stats->count >= BUDGET)
            break;
    }
}

int sane(ScanConfig * config)
{
    checkRoot();
    if (config->ipv6 and config->int_name == NULL)
        fatal("*** IPv6 requires specifying an interface!");
    if (config->pre_scan and not config->type)
        fatal("*** Pre-scan must specify the type of package!");
    if (not config->pre_scan and config->ipv6 and not config->strategy)
        fatal("*** IPv6 scanning must specify the search strategy!");
    if (config->pre_scan and config->strategy)
        fatal("*** Cannot specify pre-scan and regular scan at the same time!");
    if (BUDGET > pow(2, 32))
        fatal("*** The budget limit is 2^32!");
    return true;
}

int main(int argc, char **argv)
{
    /* Parse options */
    ScanConfig config = ScanConfig();
    config.parse_opts(argc, argv);

    /* Sanity checks */
    sane(&config);

    /* Ensure we're the only probing instance on this machine */
    instanceLock(config.instance);

    /* Setup IPv6, if using (must be done before trace object) */
    if (config.ipv6) {
        if (config.srcmac == NULL || config.dstmac == NULL) {
            LLResolv *ll = new LLResolv();
            ll->gateway();
            ll->mine(config.int_name);
            if (not config.srcmac)
                ll->setSrcMAC(&config.srcmac);
            if (not config.dstmac)
                ll->setDstMAC(&config.dstmac);
            if (config.srcmac == NULL || config.dstmac == NULL) {
                fatal("unable to auto-interpret MAC addresses; use -M, -G");
            }
        }

        /* Init IPv6 scanning engine */
        IPList6 *iplist = new IPList6();
        Stats *stats = new Stats(config.strategy);
        Traceroute6 *trace = new Traceroute6(&config, stats);
        trace->unlock();

        /* Pre-scan */
        if (config.pre_scan) {            
            string hitlist = get_hitlist(); //Read the latest hitlist in local           
            iplist->setkey(config.seed);  // Randomize permutation key
            iplist->read_hitlist(hitlist);
            cout << "Probing begins..." << endl;
            loop(&config, iplist, trace, stats);
        }

        /* IPv6 scanning with different strategy */
        if (config.strategy) {
            string type = Tr_Type_String[config.type];
            string seedset = get_seedset(type);
            iplist->setkey(config.seed);

            /* Scanning with 6Scan strategy */
            if (config.strategy == Scan6) {
                cout << "Scanning with 6Scan strategy..." << endl;
                init_6scan(stats,seedset); // Init and update prefix sketch
                stats->prepare_time(); // Calculate the time cost of preparation
                int iteration = pow(16, SUBNET_LEN);
                for (auto i = 0; i < iteration; ++i) {
                    iteration_6scan(stats, iplist, i);
                    loop(&config, iplist, trace, stats);
                    cout << "Probing in iteration: " << i+1 << ", budget consumption: " << stats->count << endl;
                    iplist->targets.clear();
                    iplist->seeded = false;
                    if (stats->count >= BUDGET)
                        break;
                }
            }

            /* Scanning with 6Tree strategy */
            else if (config.strategy == Tree6) {
                cout << "Scanning with 6Tree strategy..." << endl;
                Node_List nodelist;
                init_6tree(nodelist, iplist, seedset); // Partition the candidate scanning space
                stats->prepare_time(); // Calculate the time cost of preparation
                Node_Pri node_priority;
                for (auto iter = 1; iter < 32; ++iter) {
                    for (auto &node : nodelist) {
                        if (node->dim_num == iter) {
                            float pri = 0;
                            if (node->children_num)
                                pri = (float)node->active/(node->children_num * pow(16, iter-1));
                            node_priority.push_back(make_pair(node, pri));
                        }
                    }
                    sort(node_priority.begin(), node_priority.end(), Node_Cmp());
                    for (auto &node_pri : node_priority) {
                        stats->node_count = 0;
                        target_generation_6tree(iplist, node_pri.first->subspace, node_pri.first, 0);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        /* Quit if we've exceeded probe budget */
                        if (stats->count >= BUDGET)
                            break;
                        sleep(1); // Waiting 1s for outstanding replies...
                        if (node_pri.first->parent)
                            node_pri.first->parent->active += stats->node_count; // Update parent node's activity
                    }
                    node_priority.clear();
                    if (stats->count >= BUDGET)
                        break;
                    cout << "Probing in iteration: " << iter << ", budget consumption: " << stats->count << endl;
                }
                release_tree(nodelist);
            }

            /* Scanning with 6Gen strategy */
            else if (config.strategy == Gen6) {
                cout << "Scanning with 6Gen strategy..." << endl;
                init_6gen(iplist, seedset);
                stats->prepare_time();
                cout << "Probing begins..." << endl;
                loop(&config, iplist, trace, stats);
            }

            /* Scanning with Edgy strategy */
            else if (config.strategy == Edgy) {
            cout << "Scanning with Edgy strategy..." << endl;
            iplist->read_seedset(seedset);
            for (auto seed : iplist->seeds) {
                stats->edgy.push_back(seed);
            }
            stats->prepare_time();
            unordered_set<string> edgy_set;
            for (auto mask = 48; mask < 120; mask += 8) {
                for (auto iter : stats->edgy) {
                    edgy_set.insert(iter.substr(0, mask/4));
                }
                stats->edgy.clear();
                target_generation_edgy(iplist, edgy_set, mask);
                cout << "Probing /" << mask << "'s every /" << mask + 8 << " prefixes..." << endl;
                loop(&config, iplist, trace, stats);
                iplist->targets.clear();
                iplist->seeded = false;
                edgy_set.clear();
                if (stats->count >= BUDGET)
                    break;
                sleep(1);
                }
            }
        }

        stats->end_time();
        cout << "Waiting " << SHUTDOWN_WAIT << "s for outstanding replies..." << endl;
        sleep(SHUTDOWN_WAIT);
        if(config.strategy)
            stats->dump(config.out);
        delete iplist;
        delete trace;
        delete stats;
        cout << "End running 6Scan" << endl;
    }

    /* Classify the active addresses */
    if (config.classification) {
        cout << "Address classification..." << endl;

        uint64_t received = 0;       // Number of probes received
        uint64_t active_count = 0; // Found active addresses
        uint64_t new_count = 0; // Found new addresses
        uint64_t alias_count = 0;
        uint64_t EUI64_count = 0;
        uint64_t small_integer = 0;
        uint64_t embedded_IPv4 = 0;
        uint64_t randomized_count = 0;

        string line;
        unordered_map<string, string> results;
        vector<string> information;

        ifstream infile;
        infile.open(string(config.classification));
        while (getline(infile, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase( remove(line.begin(), line.end(), '\r'), line.end());
            if (line[0] != '#') {
                received++;
                if (results.find(line) == results.end())
                    results[line] = "other";
            } else {
                information.push_back(line);
            }
        }
        infile.close();
        active_count = results.size();

        /* Remove the seed addresses */
        string type = Tr_Type_String[config.type - 1];
        string file_name = get_seedset(type);
        string seed;
        infile.open(file_name);
        while (getline(infile, seed)) {
            if (!seed.empty() && seed[seed.size() - 1] == '\r')
                seed.erase( remove(seed.begin(), seed.end(), '\r'), seed.end());
            if (results.find(seed) != results.end())
                results.erase(seed);
        }
        infile.close();
        new_count = results.size();

        /* Alias resolution */
        Patricia *alias_tree = new Patricia(128);
        file_name = get_aliasfile();
        infile.open(file_name);
        alias_tree->populate6(infile);
        infile.close();
        int *alias = NULL;
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            alias = (int *) alias_tree->get(AF_INET6, iter->first.c_str());
            if (alias) {
                alias_count++;
                iter->second = "alias";
            }
        }
        delete alias_tree;

        /* Small-integer and EUI-64 detection */
        string::size_type idx1, idx2;
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (iter->second == "other") {
                idx1 = iter->first.find("::");
                if (idx1 != string::npos && (iter->first.size() - idx1 < 5)) {
                    iter->second = "small-integer";
                }

                idx1 = iter->first.find("ff:fe");
                if (idx1 != string::npos) {
                    idx2 = seed2vec(iter->first).find_last_of("fffe");
                    if (idx2 == 25) {
                        EUI64_count++;
                        iter->second = "EUI-64";
                    }
                }
            }
        }

        /* Embedded-IPv4 detection */
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (iter->second == "other") {
                string ipv4 = get_ipv4(iter->first);
                if (ipv4.length())
                    iter->second = ipv4;
            } else if (iter->second == "small-integer")
                small_integer++;
        }

        IPList4 *iplist = new IPList4();
        iplist->setkey(config.seed);
        Stats *stats = new Stats(0);
        Traceroute4 *trace = new Traceroute4(&config, stats);
        trace->unlock();

        iplist->read_result(results);
        if (iplist->targets.size()) {
            loop(&config, iplist, trace, stats);
            cout << "Waiting 5's for outstanding replies..." << endl;
            sleep(5);
        }

        for (auto iter1 : stats->IPv4) {
            for (auto iter2 = results.begin(); iter2 != results.end(); ++iter2) {
                if (iter1 == iter2->second) {
                    embedded_IPv4++;
                    iter2->second = "embedded-IPv4";
                }
            }
        }
        
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (isdigit((iter->second)[0]))
                iter->second = "other";
        }

        /* Randomized detection */
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (iter->second == "other") {
                string vec = seed2vec(iter->first);
                string vec1 = vec.substr(16, 8);
                string vec2 = vec.substr(24, 8);
                bitset<32> bit1(stoll(vec1, nullptr, 16));
                bitset<32> bit2(stoll(vec2, nullptr, 16));
                int count_zero = 0;
                for (auto i = 0; i < 32; ++i) {
                    if (bit1[i] == 0)
                        count_zero++;
                    if (bit2[i] == 0)
                        count_zero++;
                }
                if (count_zero >= 30 && count_zero <= 34 ) {
                    randomized_count++;
                    iter->second = "randomized";
                }
            }
        }

        /* Output the results */
        fprintf(config.out, "%-40s    %s\n", "# Discovered new addresses", "IID allocation schemes");
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            fprintf(config.out, "%-40s    %s\n", iter->first.c_str(), iter->second.c_str());
        }
        for (auto iter : information) {
            fprintf(config.out, "%s\n", iter.c_str());
        }
        fprintf(config.out, "# Received ratio: %2.2f%%\n", (float) received * 100 / BUDGET);
        fprintf(config.out, "# Hit active addresses: Number %" PRId64 ", Hit rate %2.2f%%\n", active_count, (float) active_count * 100 / BUDGET);
        fprintf(config.out, "# Discovered new addresses: Number %" PRId64 ", Discovery rate %2.2f%%\n", new_count, (float) new_count * 100 / BUDGET);
        fprintf(config.out, "# IID allocation schemas: Alias %" PRId64 " (%2.2f%%), Small-integer %" \
        PRId64 " (%2.2f%%), Randomized %" PRId64 " (%2.2f%%), Embedded-IPv4 %" PRId64 " (%2.2f%%), EUI-64 %"
        PRId64 " (%2.2f%%).\n", \
        alias_count, (float) alias_count * 100 / new_count, small_integer, (float) small_integer * 100 / new_count, \
        randomized_count, (float) randomized_count * 100 / new_count, embedded_IPv4, (float) embedded_IPv4 * 100 / new_count, \
        EUI64_count, (float) EUI64_count * 100 / new_count);
        
        fprintf(stdout, "# Received ratio: %2.2f%%\n", (float) received * 100 / BUDGET);
        fprintf(stdout, "# Hit active addresses: Number %" PRId64 ", Hit rate %2.2f%%\n", active_count, (float) active_count * 100 / BUDGET);
        fprintf(stdout, "# Discovered new addresses: Number %" PRId64 ", Discovery rate %2.2f%%\n", new_count, (float) new_count * 100 / BUDGET);
        fprintf(stdout, "# IID allocation schemas: Alias %" PRId64 " (%2.2f%%), Small-integer %" \
        PRId64 " (%2.2f%%), Randomized %" PRId64 " (%2.2f%%), Embedded-IPv4 %" PRId64 " (%2.2f%%), EUI-64 %"
        PRId64 " (%2.2f%%).\n", \
        alias_count, (float) alias_count * 100 / new_count, small_integer, (float) small_integer * 100 / new_count, \
        randomized_count, (float) randomized_count * 100 / new_count, embedded_IPv4, (float) embedded_IPv4 * 100 / new_count, \
        EUI64_count, (float) EUI64_count * 100 / new_count);

        results.clear();
        delete iplist;
        delete trace;
        delete stats;
        cout << "End running 6Scan" << endl;
    }
}