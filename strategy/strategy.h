#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "scanner/6scan.h"
#include "6tree.h"
#include "download.h"
#include "patricia.h"
#include <dirent.h>
#include <fstream>

typedef std::vector<std::pair<struct SpaceTreeNode*, float>> Node_Pri;
typedef std::pair<struct SpaceTreeNode*, float> Node_Pair;
struct Node_Cmp {
    bool operator() (const Node_Pair& p1, const Node_Pair& p2) {
        return p1.second > p2.second;
    }
};

struct Node_Dim_Cmp {
    bool operator() (struct SpaceTreeNode* p1, struct SpaceTreeNode* p2) {
        return p1->dim_num < p2->dim_num;
    }
};


void init_6scan(Node_List& nodelist, IPList6* iplist, std::string seedset);
void target_generation_6scan(IPList6* iplist, std::string subspace, int start_idx);
void init_6hit(Stats* stats, std::string in);
void iteration_6hit(Stats* stats, IPList6* iplist, int it);
void init_6tree(Node_List& nodelist, IPList6* iplist, std::string seedset);
void target_generation_6tree(IPList6* iplist, std::string subspace, struct SpaceTreeNode* node, int start_idx);
void init_Edgy(IPList6* iplist, std::string seedset);
void release_tree(Node_List& nodelist);
void init_6gen(IPList6* iplist, std::string seedset);
void target_generation_edgy(IPList6* iplist, std::unordered_set<std::string>& edgy_set, int mask);

std::string get_scan_time();
std::vector<std::string> str_split(std::string &s, const char &c);
std::string seed2vec(std::string line);
std::string vec2colon(std::string line);
std::string dec2hex(int dec, int len);
int str_cmp(std::string s1, std::string s2);
std::string get_ipv4(std::string ipv6);
using namespace std;
#endif