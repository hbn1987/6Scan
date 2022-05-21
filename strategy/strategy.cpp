/****************************************************************************
 Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "strategy.h"

Random::Random(uint32_t permsize_, ScanConfig* config) {
    permsize = permsize_;
    memset(key, 0, KEYLEN);
    int seed = 100;
    if (permsize > 5000000)
        mode = PERM_MODE_CYCLE;
    permseed(key, seed);
    perm = cperm_create(permsize, mode, PERM_CIPHER_SPECK, key, 8);
}

Random::~Random() {
    cperm_destroy(perm);
}

void Random::get_random(uint32_t iter, std::vector<std::string>& rand_vec, ScanConfig* config) {
    rand_vec.clear();
    for (auto i = 0; i < iter; ++i) {
        if (PERM_END == cperm_next(perm, &next))
            break;
        rand_vec.push_back(dec2hex(next, (config->dimension - 2)));
    }
}

Strategy::Strategy(ScanConfig* config_) {
    config = config_;
}

void Strategy::target_generation(IPList6* iplist, string subspace, int start_idx)
{
    if (iplist->targets.size() >= BUDGET )
        return;
    int idx;
    for (idx = start_idx; idx < 32; idx++)
    {
        if (subspace[idx] == '*')
        {
            break;
        }
    }
    if (idx == 32)
    {
        string ip = vec2colon(subspace) + "/128";
        iplist->subnet6(ip, iplist->targets);
        return;
    }
    for (int i = 0; i < 16; i++)
    {
        if (i < 10)
        {
            subspace[idx] = '0' + i;
        }
        else // i >= 10
        {
            subspace[idx] = 'a' + i - 10;
        }
        target_generation(iplist, subspace, idx + 1);
    }
}

/* 6Scan strategy */
int Strategy::init_6scan(Node_List& nodelist, IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);
    iplist->seeds.erase(unique(iplist->seeds.begin(), iplist->seeds.end()), iplist->seeds.end());
    tree_generation(nodelist, iplist->seeds);
    sort(nodelist.begin(), nodelist.end(), Node_Dim_Cmp());
    iplist->seeds.clear();

    int index = 0;
    for (auto& node : nodelist) {
        if (node->dim_num >= config->dimension - 3)
            break;
        index++;
    }
    return index;
}

void Strategy::update_active(Node_List& nodelist, int begin, int end) {
    for (auto i = begin; i < end; ++i) {
        if (nodelist[i]->parent)
            nodelist[i]->parent->active += nodelist[i]->active;
    }
}

/* 6Hit strategy */
void target_generation_6hit_subsequent(IPList6* iplist, std::string subspace, std::vector<std::string> rand_vec, int size) {
    vector<int> index;
    for (auto i = 0; i < 32; ++i) {
        if (subspace[i] == '*')
            index.push_back(i);
    }
    
    for (auto i = 0; i <size; ++i) {
        string ip = subspace;
        string rand_dim = rand_vec[i];
        int len = index.size() - 1;
        for (auto j : index) {
            ip[j] = rand_dim[len--];
        }
        ip = vec2colon(ip) + "/128";
        iplist->subnet6(ip, iplist->targets);
    }
}

void Strategy::target_generation_6hit(IPList6* iplist, std::string subspace, std::vector<std::string> rand_vec, int ranking, int level) {
    int rand_vec_size = rand_vec.size();
    if (ranking < level) {
        target_generation_6hit_subsequent(iplist, subspace, rand_vec, rand_vec_size);
        return;
    } else if (ranking < 2 * level) {
        target_generation_6hit_subsequent(iplist, subspace, rand_vec, rand_vec_size / 2);
        return;
    } else if (ranking < 3 * level) {
        target_generation_6hit_subsequent(iplist, subspace, rand_vec, rand_vec_size / 3);
        return;
    } else {
        target_generation_6hit_subsequent(iplist, subspace, rand_vec, rand_vec_size / 4);
    }
}

void get_prior_activity(Node_List& nodelist) {
    for (auto& node : nodelist) {
        node->active = node->upper - node->lower;
    }
}

void Strategy::partition_nodelist(Node_List& nodelist, Node_List& nodelist_small) {
    sort(nodelist.begin(), nodelist.end(), Node_Dim_Cmp());
    int index = 0;
    for (auto node : nodelist) {
        if (node->dim_num == config->dimension - 2)
            break;
        index++;
    }
    if (index >= 0) {
        nodelist_small.assign(nodelist.begin(), nodelist.begin() + index);
        nodelist.erase(nodelist.begin(), nodelist.begin() + index);
    }
}

void Strategy::init_6hit(Node_List& nodelist, Node_List& nodelist_small, IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);
    iplist->seeds.erase(unique(iplist->seeds.begin(), iplist->seeds.end()), iplist->seeds.end());
    tree_generation_6hit(nodelist, iplist->seeds);
    partition_nodelist(nodelist, nodelist_small);
    get_prior_activity(nodelist);  
    iplist->seeds.clear();
}

/* 6Tree strategy */
void Strategy::init_6tree(Node_List& nodelist, IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);
    iplist->seeds.erase(unique(iplist->seeds.begin(), iplist->seeds.end()), iplist->seeds.end());
    tree_generation(nodelist, iplist->seeds);
    sort(nodelist.begin(), nodelist.end(), Node_Dim_Cmp());
    iplist->seeds.clear();
}

int space_cmp(string ip, string space) {
    int value = 0;
    for (auto i = 0; i < 32; ++i) {
        if (space[i] == '*' || space[i] == ip[i])
            continue;
        else {
            value = 1;
            break;
        }
    }
    return value;
}

int inchild(string ip, struct SpaceTreeNode* node) {
    vector<string> children_space;
    if (node->children_num) {
        for (auto i = 0; i < node->children_num; ++i) {
            children_space.push_back(node->children[i]->subspace);
        }
        for (auto i = 0; i < children_space.size(); ++i) {
            if (space_cmp(ip, children_space[i])==0)
                return 1;
        }
    }
    return 0;
}

void Strategy::target_generation_6tree(IPList6* iplist, string subspace, struct SpaceTreeNode* node, int start_idx)
{
    int idx;
    for (idx = start_idx; idx < 32; idx++)
    {
        if (subspace[idx] == '*')
        {
            break;
        }
    }
    if (idx == 32)
    {
        if (not inchild(subspace, node)) {
            string ip = vec2colon(subspace) + "/128";
            iplist->subnet6(ip, iplist->targets);
        }
        return;
    }
    for (int i = 0; i < 16; i++)
    {
        if (i < 10)
        {
            subspace[idx] = '0' + i;
        }
        else // i >= 10
        {
            subspace[idx] = 'a' + i - 10;
        }
        target_generation_6tree(iplist, subspace, node, idx + 1);
    }
}

/* 6Gen strategy */
string merge(std::string odd, std::string even) {
    string cluster = odd;
    for (auto i = 0; i < 32; ++i) {
        if (cluster[i] != even[i])
            cluster[i] = '*';
    }
    return cluster;
}

int Strategy::get_dimension(string cluster) {
    int i = 0;
    for (auto j = 0; j < 32; ++j) {
        if (cluster[j] == '*')
            ++i;
    }
    return i;
}

void Strategy::AHC(std::vector<std::string>& even_seeds, std::vector<std::string>& odd_seeds, std::vector<std::string>& cluster_seeds, vector<std::string>& clusters, vector<std::string>& clusters_big)
{
    if (cluster_seeds.size() <= 1)
        return;

    if (cluster_seeds.size() > 1 && cluster_seeds.size() % 2 != 0) {
        cluster_seeds[cluster_seeds.size() - 2] = merge(cluster_seeds[cluster_seeds.size() - 2],
        cluster_seeds[cluster_seeds.size() - 1]);
        cluster_seeds.pop_back();
    }

    for (auto i = 0; i < cluster_seeds.size(); ++i) {
        if ( i % 2 == 0)
            even_seeds.push_back(cluster_seeds[i]);
        else
            odd_seeds.push_back(cluster_seeds[i]);
    }
    cluster_seeds.clear();

    for (auto i = 0; i < even_seeds.size(); ++i) {
        string clus = merge(odd_seeds[i], even_seeds[i]);
        int dims = get_dimension(clus);
        if (dims == config->dimension - 3)
            clusters.push_back(clus);
        else if (dims < config->dimension - 3)
            cluster_seeds.push_back(clus);
        else if (dims > config->dimension -3)
            clusters_big.push_back(clus);
    }

    even_seeds.clear();
    odd_seeds.clear();
    AHC(even_seeds, odd_seeds, cluster_seeds, clusters, clusters_big);
}

void Strategy::init_6gen(IPList6* iplist, string seedset, vector<string>& clusters, vector<std::string>& clusters_big) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);

    std::vector<std::string> even_seeds, odd_seeds, cluster_seeds;

    cluster_seeds.assign(iplist->seeds.begin(), iplist->seeds.end());
    AHC(even_seeds, odd_seeds, cluster_seeds, clusters, clusters_big);

    iplist->seeds.clear();
    even_seeds.clear();
    odd_seeds.clear();
    cluster_seeds.clear();
}

/* Herustic strategy */
void Strategy::target_generation_heuristic(IPList6* iplist, std::unordered_map<std::string, int>& prefix_map, int mask) {
    string add_zero((31 - mask/4 - 1), '0');
    for (auto& iter : prefix_map) {
        for (auto i = 0; i < 16; ++i) {
            string ip = vec2colon(iter.first + dec2hex(i, 1) + add_zero + "1") + "/128";
            iplist->subnet6(ip, iplist->targets);
        }
    }
}

void Strategy::target_generation_alias(IPList6* iplist, std::string line) {

    int pos = line.find("/");
    int num = count(line.begin(), line.end(),':');
    if (num < 8)
        line = line.substr(0, pos) + "1234/128"; // 1234 acts as a random number
    else 
        line = line.substr(0, pos - 1) + "1234/128";
    iplist->subnet6(line, iplist->targets);
}