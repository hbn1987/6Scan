#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "scanner/6scan.h"
#include "download.h"
#include "patricia.h"
#include <dirent.h>
#include <fstream>

void init_6scan(Node_List& nodelist, Node_List& nodelist_small, IPList6* iplist, std::string seedset);
void target_generation_6scan(IPList6* iplist, std::string subspace, int start_idx);
void iteration_6scan(Node_List& nodelist_sorted, Node_List& nodelist, std::unordered_set<std::string>& scanned_node);

void init_6hit(Stats* stats, std::string in);
void iteration_6hit(Stats* stats, IPList6* iplist, int it);

void init_6tree(Node_List& nodelist, IPList6* iplist, std::string seedset);
void target_generation_6tree(IPList6* iplist, std::string subspace, struct SpaceTreeNode* node, int start_idx);

void init_6gen(IPList6* iplist, std::string seedset);

void init_Edgy(IPList6* iplist, std::string seedset);
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