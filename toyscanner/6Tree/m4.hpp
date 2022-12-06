//
//  m4.hpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第4个模块，包括内容如下：
//  包括6Tree运行的第四步、第五步和第六步。第四步，根据第三步保存的生成树结构信息，构造回溯树结构，
//  并将其中的叶子节点提取出来，生成预扫描结点数组。第五步，对预扫描结点数组执行预扫描（pre-scanning），并
//  根据扫描结果生成链表。第六步，根据预扫描结果进行动态扫描（dynamic scanning），达到预算时停止。

#ifndef m4_hpp
#define m4_hpp

#include <string>
#include <cstdlib>

using namespace std;

// 第四步开始的函数

void s4_btTreeGenerate(string treefname);

int s4_cmp(string s1, string s2);

void s4_spaceInit(string canfname);

void s4_entrance();

// 第五步开始的类与函数

#if ___ENV == ___ENV_TEST
int s5_recSearch(string addr, struct seTreeNode *sit, int addr_size);
#endif

#if ___ENV == ___ENV_TEST
int s5_recIpNum(struct seTreeNode *sit);
#endif

#if ___ENV == ___ENV_TEST
void s5_seTreeGeneration();
#endif

void s5_scanPreface(string ffilename, string lfilename, string alfilename, string tfilename);

class s5_RandomGenerator
{
    // 随机数生成器
public:
    ptrdiff_t operator() (ptrdiff_t max)
    {
        double t;
        t = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        return static_cast<ptrdiff_t>(t * max);
    }
};

string s5_tranStr(string expression, int prefix);

#if ___ENV == ___ENV_WORK
string s5_tranBiStr(string addr);
#endif

#if ___ENV == ___ENV_WORK
int s5_snCmp(struct snTreeNode *node1, struct snTreeNode *node2);
#endif

void s5_ipv6Normalize(string &s1, string &s2);

bool s5_belong(string aip, struct snTreeNode *snnode);

#if ___ENV == ___ENV_WORK
string s5_tranHexStr(string addr);
#endif

#if ___ENV == ___ENV_WORK
void s5_inputAddr(string addr, int prefix);
#endif

#if ___ENV == ___ENV_WORK
string s5_tranIPv6(string addr);
#endif

void s5_recReleaseSnTree(struct snTreeNode *snnode);

#if ___ENV == ___ENV_WORK
void s5_recInputLeafAddr(string ip_addr, int start);
#endif

#if ___ENV == ___ENV_WORK
void s5_recInputAddr(struct snTreeNode *snnode);
#endif

void s5_fuseExpr(string &expression, string sexpression, string texpression);

void s5_leavesScan(int left, int right, long objects_scale);

void s5_preScan();

int s5_cmp(struct btTreeNode *node1, struct btTreeNode *node2);

void s5_sortLink();

void s5_releasePreLi();

void s5_entrance();

// 第六步开始的函数

#if ___ENV == ___ENV_TEST
void s6_recFree(struct seTreeNode *sit);
#endif

long s6_recunpnFnum(struct btTreeNode *sit);

void s6_storeTree();

void s6_scanEpilogue();

void s6_releaseLink();

void s6_releaseBtTree();

int s6_cmp(struct scanSet set1, struct scanSet set2);

int s6_unique(struct scanSet *elements, int card);

struct eLink *s6_eLinkEnd(struct eLink *start);

int s6_speRelation(struct eLink *tp, struct eLink *tq);

void s6_moveRedun(struct eLink *es);

struct eLink *s6_recScannedSpe1(struct btTreeNode *parent, long &fnum);

struct eLink *s6_recScannedSpe2(struct eLink *readySpe, struct btTreeNode *pa, int dimension, int dgreef, int level, long &fnum);

int s6_getSexpre(string &sexpression, string parent_expression, string expression);

void s6_recFinish(struct btTreeNode *parent);

void s6_releaseELink(struct eLink *link);

bool s6_isAlised(struct LinkNode *lnode);

int s6_speExpand(struct LinkNode *link);

void s6_recSubLeaf(string expression, struct snTreeNode *anode, int start_sit, int diff_num);

void s6_recSubForest(string expression, struct snTreeNode *anode);

int s6_linkScan(struct LinkNode *first, struct LinkNode *last);

void s6_insert(struct LinkNode *end, struct LinkNode *lknode);

#if ___ENV == ___ENV_WORK
void s6_addCopy(struct scanFamily &alise, struct btTreeNode *btnode);
#endif

void s6_addOutput(struct scanFamily &alise, struct btTreeNode *btnode);

#if ___ENV == ___ENV_WORK
string s6_randomAddr(struct snTreeNode **snXree);
#endif

#if ___ENV == ___ENV_WORK
void s6_aliExpand(struct LinkNode *anode);
#endif

int s6_lnCmp(struct LinkNode *ln1, struct LinkNode *ln2);

void s6_recwriteLeafAddr(struct seTreeNode *sit);

void s6_recwriteLeafSnow(string spe, struct seTreeNode *sit, int spe_size);

void s6_recwriteSnow(struct snTreeNode *snnode);

void s6_snowSta(string snowf);

void s6_dynamicScan(int ttnum, string snowf);

void s6_dynamicScan();

int s6_recstaVisualnum(struct btTreeNode *node);

void s6_storeisual();

void s6_entrance();

void s6_entrance(const char *ttnum, const char *snowf);
#endif /* m4_hpp */
