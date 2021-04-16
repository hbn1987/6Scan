#include "6scan.h"

typedef std::vector<std::pair<struct SpaceTreeNode*, float>> Node_Pri;

typedef std::pair<struct SpaceTreeNode*, float> Node_Pair;

struct Node_Cmp {
    bool operator() (const Node_Pair& p1, const Node_Pair& p2) {
        return p1.second > p2.second;
    }
};

typedef std::vector<std::pair<std::string, float>> Cluster_Pri;

typedef std::pair<std::string, float> Cluster_Pair;

struct Cluster_Cmp {
    bool operator() (const Cluster_Pair& p1, const Cluster_Pair& p2) {
        return p1.second > p2.second;
    }
};

void init_6scan(Stats* stats, std::string in);

void iteration_6scan(Stats* stats, IPList6* iplist, int it);

void init_6tree(Node_List& nodelist, IPList6* iplist, std::string seedset);

void target_generation_6tree(IPList6* iplist, std::string subspace, struct SpaceTreeNode* node, int start_idx);

void init_Edgy(IPList6* iplist, std::string seedset);

void release_tree(Node_List& nodelist);

void init_6gen(IPList6* iplist, std::string seedset);

void target_generation_edgy(IPList6* iplist, std::unordered_set<std::string>& edgy_set, int mask);