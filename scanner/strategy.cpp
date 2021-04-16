#include "6scan.h"

/* 6Hit strategy */
void init_6scan(Stats* stats, string in) {
    int sk_depth = log(BUDGET) / log(2) * 10; // number of rows in sketch
    int sk_width = log(BUDGET) / log(2) / 10 + 1; // number of column in sketch
    stats->sk = new Sketch(sk_depth, sk_width, BGP_KEY_LEN, PREFIX_KEY_LEN);
    ifstream inlist;
    inlist.open(in);
    string line;
    while (getline(inlist, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase( remove(line.begin(), line.end(), '\r'), line.end());
        stats->sk->Update(seed2vec(line));
    }
    inlist.close();
    //stats->sk->PrintAll();
}

void iteration_6scan(Stats* stats, IPList6* iplist, int it) {
    stats->sk->Getsk(stats->sk_map);
    string subnet_hex = dec2hex(it, SUBNET_LEN);
    for (auto& iter : stats->sk_map) {
        string add_zero((31 - PREFIX_KEY_LEN - SUBNET_LEN), '0');
        int mask = round(log(iter.second * BUDGET_ITERATION)/log(2)) > 0 ? 128 - round(log(iter.second * BUDGET_ITERATION)/log(2)) : 128;
        string range = vec2colon(iter.first + subnet_hex + add_zero + "1") + "/" + to_string(mask);
        iplist->subnet6(range, iplist->targets);
    }
    stats->sk_map.clear();
    //stats->sk->PrintAll();
}

/* 6Tree strategy */
void init_6tree(Node_List& nodelist, IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);
    tree_generation(nodelist, iplist->seeds);
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

void target_generation_6tree(IPList6* iplist, string subspace, struct SpaceTreeNode* node, int start_idx)
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

int get_range(char max, char min) {
    int imax, imin;
    if (max >= 'a') {
        imax = max - 'a' + 10;
    }
    else {
        imax = max - '0';
    }
    if (min >= 'a') {
        imin = min - 'a' + 10;
    }
    else {
        imin = min - '0';
    }
    return max - min + 1;
}

void release_tree(Node_List& nodelist)
{
    release_tree(nodelist[0]);
    nodelist.clear();
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

uint64_t get_range(std::string cluster) {
    int i = 0;
    for (auto j = 0; j < 32; ++j) {
        if (cluster[j] == '*')
            ++i;
    }
    return pow(16, i);
}

int clus_cmp(string s1, string s2)
{
    for (auto i = 0; i < s1.size(); ++i) {
        if (s1[i] < s2[i] && s1[i] != '*' && s2[i] != '*')
            return 1;
    }
    return 0;
}

int contain(string clus, string seed) {
    for (auto i = 0; i < 32; ++i) {
        if (clus[i] != seed[i] && clus[i] != '*') {
            return 0;
        }
    }
    return 1;
}

void AHC(std::vector<std::string>& even_seeds, std::vector<std::string>& odd_seeds,
Cluster_Pri& cluster_pri, std::vector<std::string>& seeds)
{
    if (cluster_pri.size() == 1)
        return;

    if (cluster_pri.size() > 1 && cluster_pri.size() % 2 != 0) {
        cluster_pri[cluster_pri.size() - 2].first = merge(cluster_pri[cluster_pri.size() - 2].first,
        cluster_pri[cluster_pri.size() - 1].first);
        cluster_pri.pop_back();
    }

    for (auto i = 0; i < cluster_pri.size(); ++i) {
        if ( i % 2 == 0)
            even_seeds.push_back(cluster_pri[i].first);
        else
            odd_seeds.push_back(cluster_pri[i].first);
    }
    cluster_pri.clear();

    for (auto i = 0; i < even_seeds.size(); ++i) {
        string clus = merge(odd_seeds[i], even_seeds[i]);
        if (get_range(clus)){
            uint64_t count = 0;
            float pri = 0;
            for (auto seed : seeds) {
                count += contain(clus, seed);
            }
            pri = (float) count / get_range(clus);
            cluster_pri.push_back(make_pair(clus, pri));
        }
    }

    // De-duplication with subspace ordering
    sort(cluster_pri.begin(), cluster_pri.end(), Cluster_Cmp());

    if (get_range(cluster_pri[0].first) >= BUDGET)
        return;
    else {
        even_seeds.clear();
        odd_seeds.clear();
        AHC(even_seeds, odd_seeds, cluster_pri, seeds);
    }
}

void target_generation_6gen(IPList6* iplist, string subspace, int start_idx)
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
        target_generation_6gen(iplist, subspace, idx + 1);
    }
}

void init_6gen(IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);

    Cluster_Pri cluster_pri;
    for (auto seed : iplist->seeds) {
        cluster_pri.push_back(make_pair(seed, 1));
    }

    std::vector<std::string> even_seeds, odd_seeds;
    AHC(even_seeds, odd_seeds, cluster_pri, iplist->seeds);

    target_generation_6gen(iplist, cluster_pri[0].first, 0);
    cluster_pri.clear();
}

/* Edgy strategy */
void target_generation_edgy(IPList6* iplist, std::unordered_set<std::string>& edgy_set, int mask) {
    string add_zero((31 - mask/4 - 2), '0');
    for (auto iter : edgy_set) {
        for (auto i = 0; i < 256; ++i) {
            string ip = vec2colon(iter + dec2hex(i, 2) + add_zero + "1") + "/128";
            iplist->subnet6(ip, iplist->targets);
        }
    }
}