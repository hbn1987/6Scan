//
//  definition.hpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  功能：保存定义的运行时参数、数据结构，并声明设定的全局变量。

#ifndef definition_hpp
#define definition_hpp

#include <string>
#include <fstream>

using namespace std;

// == 文件部分 ==
// 保存6Tree使用的文件名。
#define ___FL_SOURCEIPV6 "seeds" // 存放初始IPv6地址的文件
#define ___FL_IPV6 "./output/m1_norseeds" // 存放转换为16进制字符串格式的文件，种子IPv6地址，应该是排好序的，
                              // 同时字母是小写格式
#define ___FL_TEST "./output/m1_norseeds" // 当进行模拟扫描的时候，存放的所有IPv6地址，同样以16进制字符串保存，
                              // 同样应该是排好序的，同时字母是小写格式
#define ___FL_TREE "./output/m3_tree" // 保存树结构信息的文件
#define ___FL_CANIPv6 "./output/m3_candidate" // 保存候选IPv6地址的文件
#define ___FL_FIND "./output/m4_find" // 保存发现的IPv6的统计文件，分模拟扫描和实际扫描两种情况的不同，如果
                              // 是实际扫描，就会按照时间依次保存发现的IPv6地址，如果是模拟扫描，则
                              // 保存每次扫描的空间信息以及发现的活跃地址数量
#define ___FL_LIST "./output/m4_list" // 保存扫描发现的IPv6地址列表
#define ___FL_FLIST "./output/m7_flist" // 保存经过统计去重、去别名之后的IPv6地址列表
#define ___FL_ZRES "./output/m4_results" // 用来保存实时扫描结果的文件
#define ___FL_ALIAS "./output/m4_alias" // 保存发现的别名区的文件，模拟扫描下不会发现别名区
#define ___FL_TARGET "./output/m4_target" // 存放扫描目标列表的文件
#define ___FL_TEST1STR "./output/m1_norseeds" // 保存用来进行测试1的字符串
#define ___FL_TEST1RES "./output/m2_test" // 存放测试1改进汉明距离分布情况的测试文件
#define ___FL_SNOW "./output/m4_snowballing" // 存放snowballing模式下，扫描完毕后发现的所有IPv6地址
#define ___FL_FRUT "./output/m4_fruit" // 存放在扫描结束后的回溯树密度状态的文件

// == 算法参数部分 ==
// 保存6Tree算法运行过程中所要用到的一些参数，以及用于条件判断的参数。
#define ___AL_IPNUM 5000000 // 种子IPv6地址数量上限
#define ___AL_TIPNUM 5000000 // 模拟扫描中假设的所有IPv6地址数量上限
#define ___AL_BREADTH 256 // 树结构的每层广度上限，新划分方法中不再使用
#define ___AL_LEAFSCALE 16 // 叶子结点地址数上限，当结点的地址数小于这个值时，就
                           // 不再进行划分，也就是说将它作为叶子结点，其值最好不要小于16，不然在
                           // 树结构回溯等过程中可能出现问题
                           // 论文中将其明确为必须等于BASE
#define ___AL_NODESCALE 5000000 // 树结点数量上限，按照理想情况下的计算，即每次均匀划分，那么设
                                // i = BREADTH, j = IPNUM
                                // m=log_i^j的高斯取底整数，那么结点数量上限预计为
                                // i^0 + i^1 + ... + i^m = (i^0 - i^(m+1)) / (1 - i)
                                // 当i=128， j=3000000时，预计为2113665，约为2400000
                                // 通过公式估计计算，可以约为(i*j - 1) / (i - 1)，再约为j
#define ___AL_STA_UNOPED 1 // 结点状态：还未开启扫描
#define ___AL_STA_READY 2 // 结点状态：正在扫描=>待扫描，此时扫描空间信息表示待扫描空间
#define ___AL_STA_DONE 3 // 结点状态：正在扫描=>已扫描，此时扫描空间信息表示已扫描空间
#define ___AL_STA_FINISH 4 // 结点状态：已完全扫描，这个结点下的整个扫描空间已被扫描
#define ___AL_SPE_NOIN 1 // 用于地址集合之间的关系分析：无交集
#define ___AL_SPE_INC 2 // 用于地址集合之间的关系分析：A包含B
#define ___AL_SPE_INCD 3 // 用于地址集合之间的关系分析：A被B包含
#define ___AL_SCAN_THD 1000000 // 扫描工作量阈值，当累积的待扫描地址数量达到这个规模时就将累积的
                              // 地址随机化然后执行扫描
#define ___AL_SCAN_TTNUM 10000000 // 扫描地址总数，当已经扫描了这么多数量的地址之后就停止扫描
                                  // (This value is set to infinite which can scan indefinitely)
#define ___AL_ALI_BL 1 // 用于判断地址与别名区之间的关系：位于别名区下
#define ___AL_ALI_UP 2 // 用于判断地址与别名区之间的关系：位于别名区上
#define ___AL_ALI_IN 3 // 用于判断地址与别名区之间的关系：位于别名区中

// == 运行部分 ==
// 保存代码运行过程中所要用到的一些参数，与“算法参数宏定义”功能相似，但是
// 前者侧重于算法的运行，后者侧重于代码的运行。
#define ___PR_TICK 200000 // 地址处理步长
#define ___PR_PATHD 10000 // 实施并行化的规模阈值
#define ___PR_ZMAP "/usr/local/sbin/zmap" // ZMap扫描器的工作路径
#define ___PR_SOURCE "2001:da8:b6:18::2" // 扫描器自己的IPv6地址
#define ___PR_BAND "50M" // ZMap扫描其工作时的带宽
#define ___PR_ALI_BAND "1M" // 使用ZMap进行别名区时的带宽
#define ___PR_ALITEST 128 // 别名区测试时的扫描地址数量
#define ___PR_CTIME "4" // 动态扫描过程中发送完成后等待继续接收的秒数
#define ___PR_ALI_CTIME "1" // 别名区扩展过程中发送完成后等待继续接收的秒数
#define ___ENV_TEST 1 // 说明是根据测试集来进行模拟扫描，模拟扫描的时候只要地址在测试集文件中就是
                      // 活跃的
#define ___ENV_WORK 2 // 说明扫描将是实际行为，会在互联网IPv6空间中进行
#define ___ENV ___ENV_WORK
#define ___SNOW_YES 1 // 在采用模拟扫描的情况下，说明会采用snowballing模式
#define ___SNOW_NO 2 // 在采用模拟扫描的情况下，说明不采用snowballing模式
#define ___SNOW ___SNOW_YES
extern int ___al_scan_ttnum; // 扫描地址总数

// == 生成树算法部分 ==
// 保存在树的生成（tree generation）这一步骤中要用到的数据结构，主要是生成树结点（TreeNode）结构。
extern string ipv6[___AL_IPNUM]; // 存放IPv6地址的数组，其中的IPv6地址使用16进制字符串的形式表示，例如
                                 // 2001:200:0:1::3初始时表示为20010200000000010000000000000003
                                 // 假设总共有n个地址，那么其下标范围为[1...n]
extern int ipv6_scale; // IPv6地址总数
extern int dist[___AL_IPNUM]; // 用于存放改进的汉明距离，设IPv6地址数组下标范围为[1...n]，那么其下标地址为
                              // [1...n-1]，分别表示第1个地址与第2个地址之间的距离，一直到第n-1个地址与第
                              // n个地址之间的距离。这里的改进汉明距离应该就是指FDS。
struct simInfo // 归约信息（降维信息）
{
    int left; // 左下标
    int right; // 右下标
    string str; // 被归约的字符串，例如2006AF被归约为200^AF，那么可以表示为<3, 3, "6">
                // 如果被归约为2^F，那么可以表示为<1, 4, "006A">
};
struct TreeNode // 生成树结点结构
{
    int level; // 当前结点所在的层次，根结点的level为1
    int number; // 其在nodes数组中的编号
    int inf; // 地址下标下界
    int sup; // 地址下标上界，例如这个节点如果指示了ipv6数组中[i...j]位置的IPv6地址，那么inf为i，
             // sup为j
    struct simInfo sim[40]; // 结点归约信息，保存该结点下的归约情况，下标从0开始
    int simNum; // 归约信息数量
    struct TreeNode *parent; // 指向其父节点，根节点的父节点为NULL
    struct TreeNode **children; // 指向其子节点，下标从0开始
    int childrenNum; // 子节点个数
};
extern struct TreeNode Xree; // 生成树结构的起始节点
extern struct TreeNode *nodes[___AL_NODESCALE]; // 树结点指针数组，其坐标越小，保存层次越低，越靠“左”，节点
                                                // 即是按照顺序存放结点的，假设有m个结点，下标范围为[1...m]
extern int nodes_scale; // 树结点总数

// == 回溯树算法部分 ==
// 保存在回溯（backtracking）这一步骤中要用到的数据结构，主要是回溯树结点（btTreeNode）结构。
struct scanSet // 扫描集合（地址子空间）
{
    string expression; // 表达式，例如“ab*”或“a*b”，这里自由度等于1
    int dgreef; // 自由度，这里有个约束，表达式长度等于扫描空间维度
};
struct scanFamily // 扫描集族（地址子空间集合）
{
    int card; // 扫描集合的个数（地址子空间集合的基数）
    int num; // 总共包含的扫描地址数量
    struct scanSet *elements; // 保存扫描集合的数组，下标从0开始
};
struct scanSpace // 扫描空间（全局表示地址子空间）
{
    int dimension; // 扫描空间的维度
    string expression; // 表达式，例如扫描空间占3个维度，而整个字符串长度为5（注意实际上为32，这里做了简略
                       // 处理），那么表达式可以表示成“ab-c--”这样的格式，其中三个“-”符号即表示了扫描空间
                       // 所在的维度
    int charLev[32]; // 字符优先级，分别存放32个16进制字符位置所处的优先级，注意在IPv6Candidate文件中
                     // 字符优先级的信息可能保存为：13 12 12 11 11 11 11 10 10 9 9 9 8 8 8 8 7 8
                     // 7 6 6 6 6 6 5 5 4 4 3 3 2 1，接下来要将其转换为1 2 2 3 3 3 3 4 4 5 5 5
                     // 6 6 6 6 7 6 7 8 8 8 8 8 9 9 10 10 11 11 12 12，其中charLev[0] = 1，
                     // charLev[31] = 12，这样，保存了不同位置字符是在哪个层次的结点上被归约的，例如
                     // 叶子结点的level==12，那么其字符优先级就正好等于12
    struct scanFamily add, sub; // 地址空间加集族和减集族，当处于已扫描状态时仅有加集族
};
struct btTreeNode // 回溯树结点结构
{
    int level; // 当前结点所在的层次
    int number; // 其在nodes数组中的编号
    struct btTreeNode *parent; // 指向其父结点，根结点的父结点为NULL
    struct btTreeNode **children; // 指向其子结点，下标从0开始
    int childrenNum; // 子结点个数
    int status; // 结点状态，分四种：还未开启扫描（___AL_STA_UNOPED）、已完成（___AL_STA_FINISH）、
                // 正在扫描=>待扫描（___AL_STA_READY）、已扫描（___AL_STA_DONE）
                // 以下数据仅当处于正在扫描状态时才有意义
    double ratio; // 当结点处于READY状态时，ratio表示其减集族的AAP，当结点处于DONE状态时，ratio表示
                  // 其加集族的AAP
    long fnum; // 在实际扫描环境下用于linkScan函数
    struct scanSpace space; // 扫描空间信息，当处于待扫描状态时说明待扫描的空间有哪些，当处于已扫描状态
                            // 时说明已扫描的空间有哪些
    int visual_num; // 用于可视化部分，统计该结点下面的结点个数（算上它自己），叶子结点则是1。
};
extern struct btTreeNode btXree; // 回溯树结构的起始结点
extern struct btTreeNode *btnodes[___AL_NODESCALE]; // 回溯树结点指针数组，下标从1开始
extern int btnodes_scale; // 回溯树结点总数

// == 扫描算法部分 ==
// 保存在回溯这一步骤中要用到的数据结构，和“回溯树算法数据结构”相比，更侧重于代码的运行。重要的数据结构
// 包括扫描结点链表（liXree）、待扫描IPv6地址大数组（objects）、用于模拟扫描的查询树结点（seTreeNode）
// 结构、用于实际扫描的子网树结点（snTreeNode）结构。
extern struct btTreeNode **preli; // 预扫描结点指针数组，下标从0开始
extern int preli_scale; // 预扫描结点数组的规模
struct LinkNode // 结点链表，用于在预扫描之后的扫描中进行优先级排序
{
    struct btTreeNode *node; // 该链表结点下保存的回溯树结点
    struct LinkNode *next; // 该链表结点的下一结点
};
extern struct LinkNode *liXree; // 结点链表的起始结点
struct bundle
{
    int number; // 这个地址所属的回溯树结点的在btnodes中的下标
    string addr; // 这个地址的16进制表示式
};
extern struct bundle *objects; //存放待扫描IPv6地址的大数组，下标从0开始
extern ofstream results; // 用于将发现的IPv6地址数量情况输入到结果文件中，结果文件名为___FL_FIND
extern ofstream list; // 用于存放发现具体的IPv6地址，结果文件名为___FL_LIST
extern ofstream alias; // 用于将发现的别名区信息输入到该文件中，文件名为___FL_ALIAS
extern ifstream test; // 用于模拟扫描中获取假设存在的所有活跃IPv6地址
extern long tfnum; // 累积发现活跃地址的数量
extern long tsnum; // 累计扫描的数量
struct eLink // 表达式链表，用于扫描空间扩展算法
{
    string expression;
    struct eLink *next;
};

// 当进行模拟扫描的时候，将测试IPv6地址提取出来构造查询树结构
struct seTreeNode // 查询树结点结构
{
    int ipnum; // 这个结点下面有多少个模拟活跃IPv6地址
    int level; // 这个结点所在的层次，起始结点的层次为0，第一个字符所表示的结点层次为1，最后一个
    // 字符所表示的结点层次为32
    char ch; // 当前这个结点的字符，起始结点没有字符
    bool leaf; // 用于说明是否是叶子结点
    string leaf_addr; // 当该结点是叶子结点时，保存其地址表达式
    struct seTreeNode *parent; // 用于保存其父结点
    struct seTreeNode *children[16]; // 用于保存其子结点，下标从0开始，并采用下标直接索引的
    // 方式以提高查询效率，例如如果子结点有为'a'的，那么
    // children[10]就不是NULL
};
extern struct seTreeNode seXree; // 查询树结构的起始结点

// 当进行实际扫描的时候，构造子网表达式链表结点的数据结构
struct snTreeNode // 子网树结点结构
{
    string ip_addr; // 该子网的表示式，例如采用完全16进制表示时为59380**40a530000****************
    int dgreef; // 该子网的尾自由度（2进制表示），例如这里是72
    int btnode_number; // 说明这个子网结点属于哪个回溯树结点
    bool leaf; // 说明该结点是否是叶子结点
    struct snTreeNode *parent; // 该结点的父结点
    // 如果不是叶子结点，那么下面的数据结构有效：
    struct snTreeNode *children[16]; // 表示该子网的下一级子网，下标从0到15分别对应从"0"到"a"，被减集族
    // 减去的子网将被置为NULL
};
extern struct snTreeNode **snXree; // 子网森林的起始结点数组
extern int snXree_scale; // 子网森林起始结点个数
extern int snXree_tip; // 子网森林起始结点的增长步长
extern string *targetArr; // 用于保存IPv6地址的数组，下标从0开始
extern long targetArr_scale; // targetArr数组的长度
extern long targetArr_tip; // targetArr数组的增长步长
extern ofstream snow; // snowballing模式下保存IPv6地址的文件
extern string *snow_list; // 保存结果IPv6地址集合的数组，下标从0开始
extern long snow_list_scale; // snow_list数组的长度
extern long snow_list_tip; // snow_list数组的移动坐标

#endif /* definition_hpp */
