//
//  definition.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  功能：保存定义的运行时参数、数据结构，并声明设定的全局变量。

#include "definition.hpp"
#include <string>
#include <fstream>

using namespace std;

// == 运行部分 ==
int ___al_scan_ttnum; // 扫描地址总数

// == 生成树算法部分 ==
// 保存在树的生成（tree generation）这一步骤中要用到的数据结构，主要是生成树结点（TreeNode）结构。
string ipv6[___AL_IPNUM]; // 存放IPv6地址的数组，其中的IPv6地址使用16进制字符串的形式表示，例如
                          // 2001:200:0:1::3初始时表示为20010200000000010000000000000003
                          // 假设总共有n个地址，那么其下标范围为[1...n]
int ipv6_scale; // IPv6地址总数
int dist[___AL_IPNUM]; // 用于存放改进的汉明距离，设IPv6地址数组下标范围为[1...n]，那么其下标地址为
                       // [1...n-1]，分别表示第1个地址与第2个地址之间的距离，一直到第n-1个地址与第
                       // n个地址之间的距离。这里的改进汉明距离应该就是指FDS。
struct TreeNode Xree; // 生成树结构的起始节点
struct TreeNode *nodes[___AL_NODESCALE]; // 树结点指针数组，其坐标越小，保存层次越低，越靠“左”，节点
                                         // 即是按照顺序存放结点的，假设有m个结点，下标范围为[1...m]
int nodes_scale; // 树结点总数

// == 回溯树算法部分 ==
// 保存在回溯（backtracking）这一步骤中要用到的数据结构，主要是回溯树结点（btTreeNode）结构。
struct btTreeNode btXree; // 回溯树结构的起始结点
struct btTreeNode *btnodes[___AL_NODESCALE]; // 回溯树结点指针数组，下标从1开始
int btnodes_scale; // 回溯树结点总数

// == 扫描算法部分 ==
// 保存在回溯这一步骤中要用到的数据结构，和“回溯树算法数据结构”相比，更侧重于代码的运行。重要的数据结构
// 包括扫描结点链表（liXree）、待扫描IPv6地址大数组（objects）、用于模拟扫描的查询树结点（seTreeNode）
// 结构、用于实际扫描的子网树结点（snTreeNode）结构。
struct btTreeNode **preli; // 预扫描结点指针数组，下标从0开始
int preli_scale; // 预扫描结点数组的规模
struct LinkNode *liXree; // 结点链表的起始结点
struct bundle *objects; //存放待扫描IPv6地址的大数组，下标从0开始
ofstream results; // 用于将发现的IPv6地址数量情况输入到结果文件中，结果文件名为___FL_FIND
ofstream list; // 用于存放发现具体的IPv6地址，结果文件名为___FL_LIST
ofstream alias; // 用于将发现的别名区信息输入到该文件中，文件名为___FL_ALIAS
ifstream test; // 用于模拟扫描中获取假设存在的所有活跃IPv6地址
long tfnum; // 累积发现活跃地址的数量
long tsnum; // 累计扫描的数量

// 当进行模拟扫描的时候，将测试IPv6地址提取出来构造查询树结构
struct seTreeNode seXree; // 查询树结构的起始结点

// 当进行实际扫描的时候，构造子网表达式链表结点的数据结构
struct snTreeNode **snXree; // 子网森林的起始结点数组
int snXree_scale; // 子网森林起始结点个数
int snXree_tip; // 子网森林起始结点的增长坐标
string *targetArr; // 用于保存IPv6地址的数组，下标从0开始
long targetArr_scale; // targetArr数组的长度
long targetArr_tip; // targetArr数组的增长步长
ofstream snow; // snowballing模式下保存结果IPv6地址集合的文件
string *snow_list; // 保存结果IPv6地址集合的数组，下标从0开始
long snow_list_scale; // snow_list数组的长度
long snow_list_tip; // snow_list数组的移动坐标
