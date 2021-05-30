#include "strategy.h"

/* 6Scan strategy */
void get_grandfather(Node_List& nodelist) {
    for (auto& node : nodelist) {
        if (node->parent->parent) { // Father is not root
            if (node->parent->parent->dim_num == DIMENSION - 1)
                node->grandfather = node->parent->parent->subspace;
            else {
                int reduce_dim = node->parent->parent->dim_num - (DIMENSION - 1);
                string subspace = node->parent->parent->subspace;
                for (auto i = 0; i < reduce_dim; ++i) {
                    for (auto j = 0; j< 32; ++j) {
                        if (subspace[j] != node->subspace[j]) {
                            subspace[j] = node->subspace[j];
                            break;
                        }
                    }
                }
                node->grandfather = subspace;
            }
        } else { // Father is root
            string subspace = node->subspace;
            for (auto i = 0; i < 2; ++i) {
                for (auto j = 31; j >= 0; --j) {
                    if (subspace[j] != '*') {
                        subspace[j] = '*';
                        break;
                    }
                }
            }
            node->grandfather = subspace;
        }
    }
}

void partition_nodelist(Node_List& nodelist, Node_List& nodelist_small) {
    sort(nodelist.begin(), nodelist.end(), Node_Dim_Cmp());
    int index = 0;
    for (auto node : nodelist) {
        if (node->dim_num == DIMENSION - 3)
            break;
        index++;
    }
    if (index >= 0) {
        nodelist_small.assign(nodelist.begin(), nodelist.begin() + index);
        nodelist.erase(nodelist.begin(), nodelist.begin() + index);
    }
}

void init_6scan(Node_List& nodelist, Node_List& nodelist_small, IPList6* iplist, string seedset) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);
    iplist->seeds.erase(unique(iplist->seeds.begin(), iplist->seeds.end()), iplist->seeds.end());
    tree_generation_6scan(nodelist, iplist->seeds);
    partition_nodelist(nodelist, nodelist_small);
    get_grandfather(nodelist);    
    iplist->seeds.clear();
}

void target_generation_6scan(IPList6* iplist, string subspace, int start_idx)
{
    int idx;
    for (idx = start_idx; idx < 32; idx++) {
        if (subspace[idx] == '*')
            break;
    }
    if (idx == 32) {
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
        target_generation_6scan(iplist, subspace, idx + 1);
    }
}

void find_sibling(struct SpaceTreeNode* node, Node_List& nodelist, unordered_set<string>& scanned_node, int sibling_num) {
    int num = 0;
    while (node->index < 256 && num < sibling_num) {
        string var_dim = dec2hex(node->index, 2);
        string sibling = node->subspace;
        for (auto i = 0; i < 2; ++i) {
            for (auto j = 0; j < 32; ++j) {
                if (sibling[j] != node->grandfather[j]) {
                    sibling[j] = var_dim[i];
                    break;
                }
            }
        }
        node->index++;
        if (scanned_node.find(sibling) == scanned_node.end()) {
            struct SpaceTreeNode *new_node = new struct SpaceTreeNode;
            new_node->subspace = sibling;
            new_node->active = 0;
            new_node->grandfather = node->grandfather;
            new_node->index = node->index;
            nodelist.push_back(new_node);
            scanned_node.insert(sibling);
            num++;
        }
    }
}

void iteration_6scan(Node_List& nodelist_sorted, Node_List& nodelist, unordered_set<string>& scanned_node) {
    nodelist_sorted.assign(nodelist.begin(), nodelist.end());
    sort(nodelist_sorted.begin(), nodelist_sorted.end(), Node_Active_Cmp());
    int level = nodelist_sorted.size() / 4;
    for (auto i = 0; i < nodelist_sorted.size(); ++i) {
        if (nodelist_sorted[i]->active / pow(16, nodelist_sorted[i]->dim_num) > 0.9) {
            cout << "Alias alert: " << nodelist_sorted[i]->subspace << endl;
            nodelist_sorted[i]->active = 0;
            continue;
        }
        if (nodelist_sorted[i]->index < 256) {
            if (i < level) {
                find_sibling(nodelist_sorted[i], nodelist, scanned_node, 3);
                continue;
            } else if (i < 2 * level) {
                find_sibling(nodelist_sorted[i], nodelist, scanned_node, 2);
                continue;
            } else if (i < 3 * level) {
                find_sibling(nodelist_sorted[i], nodelist, scanned_node, 1);
            }
        }
    }
}

/* 6Hit strategy */
void init_6hit(Stats* stats, string in) {
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

void iteration_6hit(Stats* stats, IPList6* iplist, int it) {
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

/* 6Gen strategy */
string merge(std::string odd, std::string even) {
    string cluster = odd;
    for (auto i = 0; i < 32; ++i) {
        if (cluster[i] != even[i])
            cluster[i] = '*';
    }
    return cluster;
}

int get_dimension(string cluster) {
    int i = 0;
    for (auto j = 0; j < 32; ++j) {
        if (cluster[j] == '*')
            ++i;
    }
    return i;
}

void AHC(std::vector<std::string>& even_seeds, std::vector<std::string>& odd_seeds,
std::vector<std::string>& cluster_seeds, set<std::string>& clusters)
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
        if (dims == DIMENSION - 3)
            clusters.insert(clus);
        else if (dims < DIMENSION - 3)
            cluster_seeds.push_back(clus);
    }

    even_seeds.clear();
    odd_seeds.clear();
    AHC(even_seeds, odd_seeds, cluster_seeds, clusters);
}

void target_generation_6gen(IPList6* iplist, string subspace, int start_idx)
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
        target_generation_6gen(iplist, subspace, idx + 1);
    }
}

void init_6gen(IPList6* iplist, string seedset, set<string>& clusters) {
    iplist->read_seedset(seedset);
    sort(iplist->seeds.begin(), iplist->seeds.end(), str_cmp);

    std::vector<std::string> even_seeds, odd_seeds, cluster_seeds;

    cluster_seeds.assign(iplist->seeds.begin(), iplist->seeds.end());
    AHC(even_seeds, odd_seeds, cluster_seeds, clusters);

    iplist->seeds.clear();
    even_seeds.clear();
    odd_seeds.clear();
    cluster_seeds.clear();
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