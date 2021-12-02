/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "scanner/6scan.h"
#include "download.h"
#include "patricia.h"
#include "jsonreader.h"
#include <dirent.h>
#include <fstream>
#include <set>
class Random {
    public:
    Random(uint32_t permsize_);
    ~Random();
    void get_random(uint32_t iter, std::vector<std::string>& rand_vec);

    cperm_t* perm = NULL;
    uint32_t permsize = 0;
    PermMode mode = PERM_MODE_PREFIX;
    uint8_t key[KEYLEN];
    uint32_t next = 0;
};

void target_generation(IPList6* iplist, std::string subspace, int start_idx);

int init_6scan(Node_List& nodelist, IPList6* iplist, std::string seedset);
void update_active(Node_List& nodelist, int begin, int end);

void init_6hit(Node_List& nodelist, Node_List& nodelist_small, IPList6* iplist, std::string seedset);
void target_generation_6hit(IPList6* iplist, std::string subspace, std::vector<std::string> rand_vec, int ranking, int level);

void init_6tree(Node_List& nodelist, IPList6* iplist, std::string seedset);
void target_generation_6tree(IPList6* iplist, std::string subspace, struct SpaceTreeNode* node, int start_idx);

void init_6gen(IPList6* iplist, std::string seedset, std::vector<std::string>& clusters, std::vector<std::string>& clusters_big);

void target_generation_heuristic(IPList6* iplist, std::unordered_map<std::string, int>& prefix_map, int mask);

void target_generation_alias(IPList6* iplist, std::string line);

int get_dimension(std::string cluster);
std::string get_scan_time();
std::vector<std::string> str_split(std::string &s, const char &c);
std::string seed2vec(std::string line);
std::string vec2colon(std::string line);
std::string dec2hex(int dec, int len);
int str_cmp(std::string s1, std::string s2);
std::string get_ipv4(std::string ipv6);
using namespace std;
#endif