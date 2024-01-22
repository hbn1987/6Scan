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
        // if (stats->count >= config->budget)
        //     break;
    }
}

int sane(ScanConfig * config)
{
    checkRoot();
    if (config->ipv6 and config->int_name == NULL)
        fatal("*** IPv6 requires specifying an interface!");
    if (config->pre_scan and not config->type)
        fatal("*** Pre-scan must specify the type of package!");
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
        Stats *stats = new Stats(&config);
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

        /* Expanding seeds */
        if (config.exp_seed) { 
            string type = Tr_Type_String[config.type];  
            string seedset;
            if (config.seedfile)
                seedset = config.seedfile;
            else
                seedset = get_seedset(type);  
            cout << "Expanding the seed file: " << seedset << endl;  
            cout << "Reading the /32 prefixes from file: prefixes32" << endl;      
            iplist->setkey(config.seed); // Randomize permutation key

            std::map<string, int> prefix_num;
            int threshold = strategy->read_prefixes(PREFIXES, prefix_num, iplist, seedset);
            int index = 0, total = iplist->seeds.size(), num = config.exp_seed;
            for (auto i = prefix_num.begin(); i != prefix_num.end(); ++i) {
                index += i->second;
                if (index + num > total)
                    index = total - num;
                if (i->second < threshold) {
                    cout << "Probing in prefix: " << i->first << " using " << num << " probes" << endl;        
                    strategy->target_generation_expanding(iplist, i->first, index, num);
                    if (iplist->targets.size())
                        loop(&config, iplist, trace, stats);
                    iplist->targets.clear();
                    iplist->seeded = false;
                }
            }
        }

        /* HMap-ping6 using dynamic search and live de-aliasing techniques*/
        if (config.alias) {
            string scope = string(config.alias_range);   
            cout << "Dynamic ping and alias resolution within the scope of " << scope << endl; 
            string scope_file;
            vector<string> basicPrefixes;
            if (scope.find("country") != string::npos){ //Read the latest scope file in local       
                scope_file = get_countryfile(scope);   
                readJson(scope_file, basicPrefixes);     
            } else {
                // 文件路径
                std::string filePath = "./prefixes32";
                // 打开文件
                std::ifstream file(filePath);
                // 检查文件是否成功打开
                if (!file.is_open()) {
                    std::cerr << "无法打开文件: " << filePath << std::endl;
                    exit(-1); // 退出程序，表示错误
                }
                // 逐行读取文件内容
                std::string line;
                while (std::getline(file, line)) {
                    // 处理每一行内容
                    basicPrefixes.push_back(line);
                }
                // 关闭文件
                file.close();
            }    
            

            string type = Tr_Type_String[config.type];
            iplist->setkey(config.seed);

            for (auto& it : basicPrefixes) {
                int pos = it.find("/");
                it = seed2vec(it.substr(0, pos));
            }

            string addrtype = string(config.addrtype);
            if (addrtype != "low" and addrtype != "ran") {
                cout << "Accept only two input types: 'low' for low-byte or 'ran' for randomized." << endl;
                exit(-1);
            }

            int Alias_Threshold;
            if (addrtype == "low") 
                Alias_Threshold = 0.8;
            else
                Alias_Threshold = 0.9;

            const int groupSize = 5000;

            // 计算 vector 的大小
            std::size_t vectorSize = basicPrefixes.size();

            // 循环遍历 vector 并分组
            for (std::size_t i = 0; i < vectorSize; i += groupSize) {
                std::size_t end = std::min(i + groupSize, vectorSize);
                stats->mask = 32;
                // 当前组的元素
                std::vector<std::string> currentGroup(basicPrefixes.begin() + i, basicPrefixes.begin() + end);

                // 处理当前组的元素
                while (stats->mask <= 100) {
                        for (auto& it : currentGroup) {
                        stats->hashMap.initInsert(it.substr(0, stats->mask/4));
                    }                   
                    
                    strategy->target_generation_heuristic(iplist, stats->hashMap, stats->mask, config.probes, addrtype);
                    
                    if (iplist->targets.size())
                        loop(&config, iplist, trace, stats);
                    iplist->targets.clear();
                    iplist->seeded = false;
                    sleep(1);
                    if (addrtype == "low")
                        cout << "Probing " << stats->hashMap.nonEmptyKeyCount() << " /" << stats->mask << "'s every /" << stats->mask + 4 << " subprefixes with low-byte pattern addresses, budget consumption: " << stats->count << endl;
                    else
                        cout << "Probing " << stats->hashMap.nonEmptyKeyCount() << " /" << stats->mask << "'s every /" << stats->mask + 4 << " subprefixes with randomized pattern addresses, budget consumption: " << stats->count << endl;
                    
                    if (stats->count >= config.budget)
                        break;                
                    
                    vector<string> aliasCandicate;
                    aliasCandicate = stats->hashMap.traverse(Alias_Threshold*config.probes*16);
                    
                    // Output the candidate alias prefixes
                    // for (auto&it : aliasCandicate) {
                    //     if (it.length() >= 8){
                    //         string alias_prefix = get_alias(it, stats->mask);
                    //         stats->dump_alias(config.alias_out, alias_prefix);
                    //     }                        
                    // }

                    // if (iplist->targets.size())
                    //     loop(&config, iplist, trace, stats);
                    // iplist->targets.clear();
                    // iplist->seeded = false;
                    // sleep(1);

                    vector<string> acquirePrefxies;
                    acquirePrefxies = stats->hashMap.acquiredPrefixes();
                    stats->hashMap.clear();
                    for (auto& it : acquirePrefxies)
                        stats->hashMap.initInsert(it);
                    stats->mask += 4;
                }
            }
        }

        /* IPv6 scanning with different strategies */
        if (config.strategy and not config.alias) {
            string type = Tr_Type_String[config.type];
            string seedset;
            if (config.seedfile)
                seedset = config.seedfile;
            else
                seedset = get_seedset(type);
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
                    cout << "Probing in subspace: " << stats->nodelist[i]->subspace << ", budget consumption: " << stats->count << endl;                    
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
                        if (node->active / pow(16, node->dim_num) > 0.8) {
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
                        cout << "Probing in subspace: " << node->subspace << ", budget consumption: " << stats->count << endl;  
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
                    cout << "Probing in subspace: " << node->subspace << ", budget consumption: " << stats->count << endl;
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
                        cout << "Probing in iteration: " << i + 1 << ", within the subspace: " << node->subspace << ", budget consumption: " << stats->count << endl;
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
                        cout << "Probing in subspace: " << node->parent->subspace << ", budget consumption: " << stats->count << endl;                
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
                    cout << "Probing in subspace: " << node->subspace << ", budget consumption: " << stats->count << endl;                    
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
                        if ((*it).find('*') >= 16)
                            continue;
                        strategy->target_generation(iplist, *it, 0);
                        if (iplist->targets.size())
                            loop(&config, iplist, trace, stats);
                        iplist->targets.clear();
                        iplist->seeded = false;
                        if (stats->count >= config.budget)
                            break;
                        cout << "Probing in subspace: " << *it << ", budget consumption: " << stats->count << endl;
                    }
                }
                /* Probing big subspaces */
                for (int dim = config.dimension - 2; dim <= config.dimension; ++dim) {
                    for(vector<string>::reverse_iterator it = clusters_big.rbegin(); it != clusters_big.rend(); ++it){
                        if (strategy->get_dimension(*it) == dim && scanned_clusters.find(*it) == scanned_clusters.end()){
                            scanned_clusters.insert(*it);
                            if ((*it).find('*') >= 16)
                                continue;
                            strategy->target_generation(iplist, *it, 0);
                            if (iplist->targets.size())
                                loop(&config, iplist, trace, stats);
                            iplist->targets.clear();
                            iplist->seeded = false;
                            if (stats->count >= config.budget)
                                break;
                            cout << "Probing in subspace: " << *it << ", budget consumption: " << stats->count << endl;
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
                            if (iplist->targets.size()) {
                                loop(&config, iplist, trace, stats);
                            }
                            iplist->targets.clear();
                            iplist->seeded = false;
                            cout << "Probing in subspace: " << it << ", budget consumption: " << stats->count << endl;
                        }
                    } else if (yorn == 's') {
                        strategy->alias_detection(iter_ahc_clusters);
                        for (auto it : iter_ahc_clusters)
                            stats->dump_space(config.space_out, it, "ahc");
                        strategy->alias_detection(iter_dhc_clusters);
                        for (auto it : iter_dhc_clusters)
                            stats->dump_space(config.space_out, it, "dhc");                        
                    }              
                }
                cout << "\n";
            }
        }

        stats->end_time();
        cout << "Waiting " << SHUTDOWN_WAIT << "s for outstanding replies..." << endl;
        sleep(SHUTDOWN_WAIT);
        if (config.pre_scan or config.exp_seed) {
            float t = (float) tsdiff(&stats->end, &stats->start) / 1000.0;
            cout << "Budget consumption: " << (float) stats->count /1000000 << "M, Time cost: " << t << "s, Probing rate: " << (float) stats->count / t << "pps" << endl;
        } else if (config.strategy) {
            if (config.strategy != Heuristic)
                stats->dump(config.out);
            else
                stats->dumpHeuristic(config.out);
        }

        delete iplist;
        delete trace;
        delete stats;
        cout << "End running 6Scan" << endl;
    }

    /* Classify the active addresses */
    if (config.classification) {
        cout << "Aliased addresses resolution..." << endl;

        uint64_t received = 0;       // Number of probes received
        uint64_t unique = 0; // Unique active addresses
        uint64_t alias_count = 0; // Aliased addresses

        ifstream infile;        

        Patricia *alias_tree = new Patricia(128);

        string file_name = get_aliasfile(); // Get alias prefix list
        cout << "Remove alias addresses using file " << file_name << endl;
        infile.open(file_name);
        alias_tree->populate6(infile);
        infile.close();

        int *alias = NULL;
        string line;
        unordered_map<string, string> results;

        infile.open(string(config.classification));
        while (getline(infile, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase( remove(line.begin(), line.end(), '\r'), line.end());
            if (line[0] != '#') {
                std::string responder;
                received++;
                size_t found = line.find(',');
                if (found != std::string::npos) {
                    size_t startPos = line.find_first_of(",") + 1;
                    size_t endPos = line.find(",", startPos);    
                    responder = trim(line.substr(startPos, endPos - startPos));
                } else {
                    responder = line;
                }                
                if (results.find(responder) == results.end()){
                    alias = (int *) alias_tree->get(AF_INET6, responder.c_str());
                    if (alias) {
                        alias_count++;
                    } else {results[responder] = line;}
                }
            }
        }
        infile.close();       
       
        unique = results.size();       
        delete alias_tree;

        /* Output the results without classification*/
        for (auto iter = results.begin(); iter != results.end(); ++iter) {
            // fprintf(stdout, "%s\n", iter->second.c_str());
            fprintf(config.out, "%s\n", iter->second.c_str());
        }

        fprintf(stdout, "# Total received replies %" PRId64 "\n", received);
        fprintf(stdout, "# Alias addresses %" PRId64 "\n", alias_count);
        fprintf(stdout, "# Non-alias addresses %" PRId64 "\n", unique);

        results.clear();
        cout << "End running 6Scan" << endl;
    }
}