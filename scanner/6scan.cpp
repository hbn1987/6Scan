/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
/****************************************************************************
 Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
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

        if (not config->pre_scan and config->strategy != Heuristic) {
            if (stats->budgets_list.size()) {
                if (stats->count >= stats->budgets_list[0]) {
                    stats->dump_budget(config->out);
                    stats->erase_budget();
                }
            }
        }        

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
        if (stats->count >= config->budget)
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
    if (not config->alias_range and not config->pre_scan and config->ipv6 and not config->strategy)
        fatal("*** IPv6 scanning must specify the search strategy!");
    if (config->pre_scan and config->strategy)
        fatal("*** Cannot specify pre-scan and regular scan at the same time!");
    if (config->budget > pow(2, 32))
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
        Strategy *strategy = new Strategy(&config);
        trace->unlock();

        /* Pre-scan */
        if (config.pre_scan) {   
            string hitlist;
            if (config.seedfile)
                hitlist = string(config.seedfile);
            else
                hitlist = get_hitlist(); //Read the latest hitlist in local   
            cout << "Start pre-scanning the hitlist file: " << hitlist << endl;        
            iplist->setkey(config.seed); // Randomize permutation key
            iplist->read_hitlist(hitlist);
            cout << "Probing begins..." << endl;
            loop(&config, iplist, trace, stats);
        }

        /* Active search and alias resolution within a scope using heuristic strategy*/
        if (config.alias) {
            string scope = string(config.alias_range);   
            cout << "Active search and alias resolution within the scope of " << scope << endl; 
            string scope_file;
            if (scope.find("country") != string::npos){ //Read the latest scope file in local       
                scope_file = get_countryfile(scope);       
            } else {
                scope_file = get_asfile(scope);
            }
            readJson(scope_file, stats->prefixes); 

            string type = Tr_Type_String[config.type];
            iplist->setkey(config.seed);

            if (scope.find("country") != string::npos){ // 32-bit mask for national-level and 48-bit mask for AS level     
                stats->mask = 32;       
            } else {
                stats->mask = 48;
            }

            for (auto& it : stats->prefixes) {
                int pos = it.find("/");
                it = seed2vec(it.substr(0, pos));
            }

            while (stats->mask <= 112) {
                for (auto& it : stats->prefixes) {
                    stats->prefix_map.insert(pair<string, int>{it.substr(0, stats->mask/4), 0});
                    if (stats->prefix_map.size() >= 30000)
                        break;
                }

                strategy->target_generation_heuristic(iplist, stats->prefix_map, stats->mask);
                
                if (iplist->targets.size())
                    loop(&config, iplist, trace, stats);
                iplist->targets.clear();
                iplist->seeded = false;
                sleep(1);
                cout << "\rProbing " << stats->prefix_map.size() << " /" << stats->mask << "'s every /" << stats->mask + 4 << " subprefixes with host address of ::1, budget consumption: " << stats->count;
                
                if (stats->count >= config.budget)
                    break;                
                
                cout << "\rCandicate alias-prefix resolution with mask of " << stats->mask << " ...";
                unordered_map<string, int>::iterator it = stats->prefix_map.begin();
                while (it != stats->prefix_map.end()) {
                    if (it->second > 10) {
                        for (auto i = 0; i < stats->prefixes.size(); ++i) { // Radical deletion of possible alias prefixes
                            if (stats->prefixes[i].find(it->first) != string::npos)
                                stats->prefixes.erase(stats->prefixes.begin() + i);
                        }                      
                        strategy->target_generation_alias(iplist, get_alias(it->first, stats->mask));
                        it++;
                    } else {                        
                        stats->prefix_map.erase(it++);
                    }
                }

                if (iplist->targets.size())
                    loop(&config, iplist, trace, stats);
                iplist->targets.clear();
                iplist->seeded = false;
                sleep(1);

                for (auto& it : stats->prefix_map) {
                    if (it.second > 16) {                        
                        string alias_prefix = get_alias(it.first, stats->mask);
                        stats->dump_alias(config.alias_out, alias_prefix);
                    }
                }
                stats->prefix_map.clear();
                stats->mask += 4;
            }
        }

        /* IPv6 scanning with different strategies */
        if (config.strategy and not config.alias) {
            string type = Tr_Type_String[config.type];
            string seedset;
            if (config.seedfile)
                seedset = config.seedfile;
            else
                seedset = get_seedset(type, config.region_limit);
            iplist->setkey(config.seed);

            /* Scanning with 6Scan strategy */
            if (config.strategy == Scan6) {
                cout << "Scanning with 6Scan strategy..." << endl;
                int bound = strategy->init_6scan(stats->nodelist, iplist, seedset); // Partition the candidate scanning space
                stats->prepare_time(); // Calculate the time cost of preparation
                for (auto i = 0; i < bound; ++i) {
                    trace->change_fingerprint(i);                    
                    strategy->target_generation_6tree(iplist, stats->nodelist[i]->subspace, stats->nodelist[i], 0);
                    if (iplist->targets.size())
                        loop(&config, iplist, trace, stats);
                    iplist->targets.clear();
                    iplist->seeded = false;
                    if (stats->count >= config.budget)
                        break;
                    cout << "\rProbing in subspace: " << stats->nodelist[i]->subspace << ", budget consumption: " << stats->count;                    
                }

                strategy->update_active(stats->nodelist, 0, bound);
                Node_List node_priority;
                for (auto dim = config.dimension - 3; dim <= config.dimension + 1; ++dim) {
                    int lastbound = bound;
                    node_priority.clear();
                    for (auto i = bound; i < stats->nodelist.size(); ++i) {
                        if (stats->nodelist[i]->dim_num == dim) {
                            node_priority.push_back(stats->nodelist[i]);
                            bound++;
                        } else if (stats->nodelist[i]->dim_num > dim)
                            break;
                    }
                    sort(node_priority.begin(), node_priority.end(), Node_Active_Cmp());
                    for (auto& node : node_priority) {
                        if (node->active / pow(16, node->dim_num) > Alias_Threshold) {
                            cout << "Alias alert: " << node->subspace << endl;
                            continue;
                        }
                        Node_List::iterator it = find(stats->nodelist.begin(), stats->nodelist.end(), node);
                        uint64_t index = distance(stats->nodelist.begin(), it);
                        trace->change_fingerprint(index);
                        strategy->target_generation_6tree(iplist, node->subspace, node, 0);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        if (stats->count >= config.budget)
                            break;
                        cout << "\rProbing in subspace: " << node->subspace << ", budget consumption: " << stats->count;  
                    }
                    if (stats->count >= config.budget)
                        break;
                    strategy->update_active(stats->nodelist, lastbound, bound);
                }
            }

            /* Scanning with 6Hit strategy */
            else if (config.strategy == Hit6) {
                cout << "Scanning with 6Hit strategy..." << endl;
                Node_List nodelist_small; // A node list sorted by the activeity of nodes
                strategy->init_6hit(stats->nodelist, nodelist_small, iplist, seedset); // Partition the candidate scanning space
                stats->prepare_time(); // Calculate the time cost of preparation

                for (auto& node : nodelist_small) {
                    strategy->target_generation(iplist, node->subspace, 0);
                    if (iplist->targets.size())
                        loop(&config, iplist, trace, stats);
                    iplist->targets.clear();
                    iplist->seeded = false;
                    cout << "\rProbing in subspace: " << node->subspace << ", budget consumption: " << stats->count;
                }

                Random rand = Random(pow(16, config.dimension - 2), &config);
                vector<string> rand_vec;
                Node_List node_priority;
                node_priority.assign(stats->nodelist.begin(), stats->nodelist.end());

                int iteration = config.dimension * 100;
                uint32_t num_per_iter = pow(16, config.dimension - 2) / iteration;
                int level = stats->nodelist.size() / 4;

                for (auto i = 0; i < iteration; ++i) {                    
                    sort(node_priority.begin(), node_priority.end(), Node_Active_Cmp());
                    rand.get_random(num_per_iter, rand_vec, &config);

                    int ranking = 0;
                    for (auto& node : node_priority) {
                        Node_List::iterator it = find(stats->nodelist.begin(), stats->nodelist.end(), node);
                        uint64_t index = distance(stats->nodelist.begin(), it);
                        trace->change_fingerprint(index); // Change fingerprint
                        strategy->target_generation_6hit(iplist, node->subspace, rand_vec, ranking, level);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        ranking++;
                        if (stats->count >= config.budget)
                            break;
                        cout << "\rProbing in iteration: " << i + 1 << ", within the subspace: " << node->subspace << ", budget consumption: " << stats->count;
                    }                               
                }
                /* Probing big subspace */
                set<string> scanned_subspace;
                sort(node_priority.begin(), node_priority.end(), Node_Active_Cmp());
                for(auto& node : node_priority) {
                    if ((node->parent->dim_num >= config.dimension - 1 && node->parent->dim_num <= config.dimension + 1) && scanned_subspace.find(node->parent->subspace) == scanned_subspace.end()) {
                        scanned_subspace.insert(node->parent->subspace);
                        strategy->target_generation_6tree(iplist, node->parent->subspace, node->parent, 0);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        if (stats->count >= config.budget)
                            break;
                        cout << "\rProbing in subspace: " << node->parent->subspace << ", budget consumption: " << stats->count;                
                    }
                    if (stats->count >= config.budget)
                        break;
                }
                scanned_subspace.clear();
            }

            /* Scanning with 6Tree strategy */
            else if (config.strategy == Tree6) {
                cout << "Scanning with 6Tree strategy..." << endl;
                strategy->init_6tree(stats->nodelist, iplist, seedset); // Partition the candidate scanning space
                stats->prepare_time(); // Calculate the time cost of preparation
                for (auto &node : stats->nodelist) {                    
                    strategy->target_generation_6tree(iplist, node->subspace, node, 0);
                    if (iplist->targets.size())
                        loop(&config, iplist, trace, stats);
                    iplist->targets.clear();
                    iplist->seeded = false;
                    if (stats->count >= config.budget)
                        break;
                    cout << "\rProbing in subspace: " << node->subspace << ", budget consumption: " << stats->count;                    
                }
            }

            /* Scanning with 6Gen strategy */
            else if (config.strategy == Gen6) {
                cout << "Scanning with 6Gen strategy..." << endl;
                vector<string> clusters, clusters_big;
                set<string> scanned_clusters;
                strategy->init_6gen(iplist, seedset, clusters, clusters_big);
                stats->prepare_time();
                for (vector<string>::reverse_iterator it = clusters.rbegin(); it != clusters.rend(); ++it) {
                    if (scanned_clusters.find(*it) == scanned_clusters.end()) {
                        scanned_clusters.insert(*it);
                        strategy->target_generation(iplist, *it, 0);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        if (stats->count >= config.budget)
                            break;
                        cout << "\rProbing in subspace: " << *it << ", budget consumption: " << stats->count;
                    }
                }
                /* Probing big subspaces */
                for (int dim = config.dimension - 2; dim <= config.dimension; ++dim) {
                    for(vector<string>::reverse_iterator it = clusters_big.rbegin(); it != clusters_big.rend(); ++it){
                        if (strategy->get_dimension(*it) == dim && scanned_clusters.find(*it) == scanned_clusters.end()){
                            scanned_clusters.insert(*it);
                            strategy->target_generation(iplist, *it, 0);
                            if (iplist->targets.size())
                                loop(&config, iplist, trace, stats);
                            iplist->targets.clear();
                            iplist->seeded = false;
                            if (stats->count >= config.budget)
                                break;
                            cout << "\rProbing in subspace: " << *it << ", budget consumption: " << stats->count;
                        }
                    }                
                }
                clusters.clear();
                clusters_big.clear();
                scanned_clusters.clear();
            }
            /* Scanning with HMap6 strategy */
            else if (config.strategy == HMap6) {
                cout << "Scanning with HMap6 strategy..." << endl;
                int dim;
                cout << "Enter the dimensions you want to expand the scan from the seeds:" << endl;
                cin >> dim;
                config.dimension = dim;
                vector<string> ahc_clusters, dhc_clusters;
                strategy->init_hmap6(iplist, seedset, ahc_clusters, dhc_clusters);                
                stats->prepare_time();

                set<string> iter_ahc_clusters, iter_dhc_clusters, scan_clusters;
                char yorn;
                for (int i = 1; i <= dim; ++i) {
                    scan_clusters.clear();
                    iter_ahc_clusters.clear();
                    iter_dhc_clusters.clear(); 
                    strategy->get_fit_cluster(ahc_clusters, dhc_clusters, iter_ahc_clusters, iter_dhc_clusters, i);
                    cout << "Number of DHC-subspaces with " << i << " variable dimension(s): " << iter_dhc_clusters.size() << \
                    "; number of AHC-subspaces with " << i << " variable dimension(s): " << iter_ahc_clusters.size() << endl;
                    set_union(iter_ahc_clusters.begin(), iter_ahc_clusters.end(), iter_dhc_clusters.begin(), iter_dhc_clusters.end(), inserter(scan_clusters, scan_clusters.begin()));  
                    strategy->alias_detection(scan_clusters);  
                    cout << "Number of subspaces with " << i << " variable dimension(s): " << scan_clusters.size() << \
                    ", this will consume a budget of " << scan_clusters.size() * pow(16, i) / 1000000 << "M, whether to continue (y/n)?" << endl;
                    cin >> yorn;         
                    if (yorn == 'n')
                        continue;
                    else if (yorn == 'y') { 
                        for (auto it : scan_clusters) {                         
                            strategy->target_generation(iplist, it, 0);
                            if (iplist->targets.size())
                                loop(&config, iplist, trace, stats);
                            iplist->targets.clear();
                            iplist->seeded = false;
                            cout << "\rProbing in subspace: " << it << ", budget consumption: " << stats->count;
                        }
                    }              
                }
                cout << "\n";
            }
        }

        stats->end_time();
        cout << "\rWaiting " << SHUTDOWN_WAIT << "s for outstanding replies..." << endl;
        sleep(SHUTDOWN_WAIT);
        if(config.pre_scan) {
            float t = (float) tsdiff(&stats->end, &stats->start) / 1000.0;
            cout << "Time cost: " << t << "s, Probing rate: " << (float) stats->count / t << "pps" << endl;
        } else if(config.strategy)
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
        uint64_t alias_count = 0; // Aliased addresses
        uint64_t EUI64_count = 0; // EUI64 addresses
        uint64_t small_integer = 0; // Small_integer addresses
        uint64_t embedded_IPv4 = 0; // Embedded IPv4 addresses
        uint64_t randomized_count = 0; // Randomized addresses

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
        cout << "Removing the seed addresses..." << endl;
        string file_name;
        if (config.type) { // If no type is specified, the entire hitlist is excluded
            string type = Tr_Type_String[config.type - 1];
            file_name = get_seedset(type, config.region_limit);
        } else {
            file_name = get_hitlist();
        }
        
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
        cout << "Aliased addresses resolution..." << endl;
        Patricia *alias_tree = new Patricia(128);
        file_name = get_aliasfile(); // Gasser's alias prefix list
        infile.open(file_name);
        alias_tree->populate6(infile);
        infile.close();

        if (config.region_limit) {
            vector<string> aliases;
            get_aliasfile_all(aliases);
            for (auto& it : aliases) {
                if (it.find(string(config.region_limit)) != string::npos)
                    file_name = it;
            }
            infile.open(file_name);
            alias_tree->populate6(infile);
            infile.close();
        }

        int *alias = NULL;
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            alias = (int *) alias_tree->get(AF_INET6, iter->first.c_str());
            if (alias) {
                alias_count++;
                iter->second = "alias";
            }
        }
        delete alias_tree;

        new_count = new_count - alias_count; // De-alias

        /* Small-integer and EUI-64 detection */
        cout << "Small-integer and EUI-64 detection..." << endl;
        string::size_type idx1, idx2;
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (iter->second == "other") {
                idx1 = iter->first.find("::");
                if (idx1 != string::npos) {
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
        cout << "Embedded-IPv4 addresses detection..." << endl;
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

        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (isdigit((iter->second)[0])) {
                if (stats->IPv4.find(iter->second) != stats->IPv4.end()) {
                    embedded_IPv4++;
                    iter->second = "embedded-IPv4";
                    stats->IPv4.erase(iter->second);
                }
            }            
        }
        
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (isdigit((iter->second)[0]))
                iter->second = "other";
        }

        /* Randomized detection */

        randomized_count = new_count - small_integer - embedded_IPv4 - EUI64_count;

        /* Output the results without classification*/
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            if (iter->second != "alias")
                fprintf(config.out, "%s\n", iter->first.c_str());
        }

        fprintf(stdout, "# Received ratio: %2.2f%%\n", (float) received * 100 / config.budget);
        fprintf(stdout, "# Alias addresses %" PRId64 "\n", alias_count);
        fprintf(stdout, "# Discovered new addresses: Number %" PRId64 ", Hit rate %2.2f%%\n", new_count, (float) new_count * 100 / config.budget);
        fprintf(stdout, "# IID allocation schemas: Small-integer %" \
        PRId64 " (%2.2f%%), Randomized %" PRId64 " (%2.2f%%), Embedded-IPv4 %" PRId64 " (%2.2f%%), EUI-64 %"
        PRId64 " (%2.2f%%)\n", small_integer, (float) small_integer * 100 / new_count, \
        randomized_count, (float) randomized_count * 100 / new_count, embedded_IPv4, (float) embedded_IPv4 * 100 / new_count, \
        EUI64_count, (float) EUI64_count * 100 / new_count);

        fprintf(config.out, "# Received ratio: %2.2f%%\n", (float) received * 100 / config.budget);
        fprintf(config.out, "# Alias addresses %" PRId64 "\n", alias_count);
        fprintf(config.out, "# Discovered new addresses: Number %" PRId64 ", Hit rate %2.2f%%\n", new_count, (float) new_count * 100 / config.budget);
        fprintf(config.out, "# IID allocation schemas: Small-integer %" \
        PRId64 " (%2.2f%%), Randomized %" PRId64 " (%2.2f%%), Embedded-IPv4 %" PRId64 " (%2.2f%%), EUI-64 %"
        PRId64 " (%2.2f%%)\n", small_integer, (float) small_integer * 100 / new_count, \
        randomized_count, (float) randomized_count * 100 / new_count, embedded_IPv4, (float) embedded_IPv4 * 100 / new_count, \
        EUI64_count, (float) EUI64_count * 100 / new_count);

        results.clear();
        delete iplist;
        delete trace;
        delete stats;
        cout << "End running 6Scan" << endl;
    }

    if (config.dealias) {
        cout << "De-alias the regional hitlist..." << endl;        

        uint64_t active_count = 0; // Found active addresses
        uint64_t new_count = 0; // Found new addresses
        uint64_t alias_count = 0; // Aliased addresses

        string line;
        unordered_set<string> results;

        ifstream infile;
        infile.open(string(config.dealias));
        while (getline(infile, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase( remove(line.begin(), line.end(), '\r'), line.end());
            results.insert(line);
        }
        infile.close();
        active_count = results.size();

        string file_name;

        /* Alias resolution */
        cout << "Aliased addresses resolution..." << endl;
        Patricia *alias_tree = new Patricia(128);
        file_name = get_aliasfile(); // Gasser's alias prefix list
        infile.open(file_name);
        alias_tree->populate6(infile);
        infile.close();

        if (config.region_limit) {
            vector<string> aliases;
            get_aliasfile_all(aliases);
            for (auto& it : aliases) {
                if (it.find(string(config.region_limit)) != string::npos)
                    file_name = it;
            }
            infile.open(file_name);
            alias_tree->populate6(infile);
            infile.close();
        }

        ofstream file_writer(config.dealias, ios_base::out);

        int *alias = NULL;
        for (auto& iter : results) {
            alias = (int *) alias_tree->get(AF_INET6, iter.c_str());
            if (alias)
                alias_count++;
            else
                file_writer << iter << "\n";
        }
        file_writer.close();
        delete alias_tree;

        new_count = active_count - alias_count; // De-alias
        cout << "Number of alias addresses: " << alias_count << endl;
        cout << "Number of seed addresses: " << new_count << endl;
    }
}