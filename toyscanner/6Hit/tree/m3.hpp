//
//  m3.hpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第3个模块，包括内容如下：
//  包括6Tree运行的第二步和第三步。第二步，将第一步中转换好了的IPv6地址提取出来保存到地址向量数
//  组中。第三步，从IPv6地址向量数组开始进行6Tree的树的生成（tree generation），并保存在生成树结构中，
//  然后生成树结构的信息将以文件的形式保存下来。

#ifndef m3_hpp
#define m3_hpp

#include <string>

using namespace std;

// 第二步开始使用的函数

void s2_getIPv6(string filename);

void s2_entrance(const char *fn);

void s2_entrance();

// 第三步开始使用的函数

// 将t与s的字符依次进行比较，如果存在不同就将t中的对应字符更改为'-'，已经变为'-'的字符不再比较
void s3_strCompare(string &t, string s);

// 从t中找出非'-'的字符，然后将ip中对应的位置去除掉。例如，ip="A1234"， t="A--3-"，
// 那么reduce(ip, t)="124"
void s3_reduce(string &ip, string t);

// 向node指向的结点添加子结点
void s3_addNode(int left, int right, struct TreeNode *node);

// 根据下界inf、上界sup、划分阈值rank、结点指针node来实施划分
void s3_divide(int inf, int sup, int rank, int sum, struct TreeNode *node);

// 计算[inf, sup]范围内的维相似度（DS）除前维相似度（FDS）
double s3_dsfds(int inf, int sup);

// 6Tree的生成树（tree generation）部分
void s3_treeGenerate();

// 保存树的生成（tree generation）运算结果
void s3_canGenerate(string treefname, string canfname);

// 释放生成的数据结构结构
void s3_releaseTree();

// 第三步的入口函数
void s3_entrance(string output);

#endif /* m3_hpp */
