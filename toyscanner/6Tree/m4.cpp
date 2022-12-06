//
//  m4.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第4个模块，包括内容如下：
//  包括6Tree运行的第四步、第五步和第六步。第四步，根据第三步保存的生成树结构信息，构造回溯树结构，
//  并将其中的叶子节点提取出来，生成预扫描结点数组。第五步，对预扫描结点数组执行预扫描（pre-scanning），并
//  根据扫描结果生成链表。第六步，根据预扫描结果进行动态扫描（dynamic scanning），达到预算时停止。

#include "m1.hpp"
#include "m4.hpp"
#include "definition.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

// 第四步开始使用的函数

void s4_btTreeGenerate(string treefname)
{
    // 根据保存生成树信息和候选地址（叶子结点）信息的文件，生成回溯树结构
    cout << "Step 4.1: Generate Backtracking Tree" << endl;
    // 1. 初始化回溯树结构起始结点
    btXree.level = 1;
    btXree.number = 1;
    btXree.parent = NULL;
    btXree.children = NULL;
    btXree.childrenNum = 0;
    btXree.status = ___AL_STA_UNOPED;
    btXree.ratio = 0.0;
    btXree.space.dimension = 0;
    btXree.space.expression = "";
    btXree.space.add.card = 0;
    btXree.space.add.num = 0;
    btXree.space.add.elements = NULL;
    btXree.space.sub.card = 0;
    btXree.space.sub.num = 0;
    btXree.space.sub.elements = NULL;
    
    // 2. 将起始结点放入btnodes的第一个位置
    btnodes_scale = 1;
    btnodes[btnodes_scale] = &btXree;
    btnodes[0] = NULL;
    
    // 3. 从生成树信息文件提取信息，构造回溯树结构和结点数组
    ifstream infile;
    string line;
    infile.open(treefname);
    // 跳过模板部分
    for (int i = 0; i < 11; i++)
    {
        getline(infile, line);
    }
    // 从这里开始即是正式的树结构信息
    int tick = 1;
    preli_scale = 0;
    for (int i = 1; i <= btnodes_scale; i++)
    {
        // 对非叶子节点也初始化扫描空间信息:
        // 对非叶子节点初始化的扫描空间信息，仅保存在dimension，expression这两个数据，
        // 除此之外，charLev以及add和sub数据均为未初始化的无效数据。并且特别要注意的是，
        // UNOPEND状态下的非叶子结点保存的dimension和expression实际上正好是其子结点
        // “扩展满”时的状态，例如简约处理后假设整个字符串长度为5，UNOPEND结点的dimension
        // 为2，expression为abc--，那么实际上在其子结点“扩展满”的时候，地址空间才正好
        // 是abc**，也是该结点第一次启用时的地址空间状态，然而此时的结点expression就不是
        // abc--了，而可能是a----，同时dimension为4，而“bc”保存在了其x+空间中。
        struct btTreeNode *btnode;
        btnode = btnodes[i];
        getline(infile, line);
        int level;
        int number;
        getline(infile, line);
        sscanf(line.c_str(), "Location: <%d, %d>", &level, &number);
        getline(infile, line);
        int simNum;
        struct simInfo simInfo[40];
        getline(infile, line);
        sscanf(line.c_str(), "SimNum: %d", &simNum);
        if (simNum > 0)
        {
            getline(infile, line); // SimInfo:
        }
        for (int j = 0; j < simNum; j++)
        {
            int tmp;
            int left;
            int right;
            char tstr[32];
            string str;
            getline(infile, line);
            sscanf(line.c_str(), "%d: <%d, %d, %[0-9A-Za-z]>", &tmp, &left, &right, tstr);
            str = tstr;
            simInfo[j].left = left;
            simInfo[j].right = right;
            simInfo[j].str = str;
        }
        int parent_number;
        getline(infile, line);
        sscanf(line.c_str(), "Parent: %d", &parent_number);
        int childrenNum;
        getline(infile, line);
        sscanf(line.c_str(), "ChildrenNum: %d", &childrenNum);
        // 根据归约信息，初始化btnode->space.dimension和expression数据
        // 归约信息现在保存在了simNum和simInfo[40]里面
        btnode->level = level;
        btnode->number = number;
        btnode->parent = btnodes[parent_number];
        btnode->childrenNum = childrenNum;
        btnode->children = NULL;
        btnode->status = ___AL_STA_UNOPED;
        btnode->ratio = 0.0;
        btnode->space.add.card = 0;
        btnode->space.add.num = 0;
        btnode->space.add.elements = NULL;
        btnode->space.sub.card = 0;
        btnode->space.sub.num = 0;
        btnode->space.sub.elements = NULL;
        btnodes_scale += childrenNum;
        if (btnode->parent == NULL)
        {
            // 如果是根节点，直接生成空间信息
            btnode->space.expression = "--------------------------------";
            int tdm = 0;
            for (int j = 0; j < simNum; j++)
            {
                int l = simInfo[j].left;
                int r = simInfo[j].right;
                string strr = simInfo[j].str;
                int lengt = r - l + 1;
                tdm += lengt;
                for (int k = 0; k < lengt; k++)
                {
                    (btnode->space.expression)[k + l] = strr[k];
                }
            }
            btnode->space.dimension = 32 - tdm;
        }
        else
        {
            // 否则，提取其父节点的信息，然后在其基础上生成空间信息
            // 方法：首先动态生成一个维度是32-p_dimen的'-'字符串，
            // 然后把本节点的归约信息加到这个字符串上面，然后把这个字符串再
            // 加到父结点字符串上面，及是正确的字符串
            int p_dimen = btnode->parent->space.dimension;
            string p_expre = btnode->parent->space.expression;
            string s_expre = "";
            for (int j = 0; j < p_dimen; j++)
            {
                s_expre += "-";
            }
            int tdm = 0;
            for (int j = 0; j < simNum; j++)
            {
                int l = simInfo[j].left;
                int r = simInfo[j].right;
                string strr = simInfo[j].str;
                int lengt = r - l + 1;
                tdm += lengt;
                for (int k = 0; k < lengt; k++)
                {
                    s_expre[k + l] = strr[k];
                }
            }
            btnode->space.dimension = p_dimen - tdm;
            int st = 0;
            for (int j = 0; j < 32; j++)
            {
                if (p_expre[j] == '-')
                {
                    p_expre[j] = s_expre[st++];
                }
            }
            btnode->space.expression = p_expre;
        }
        
        if (childrenNum > 0)
        {
            btnode->children = new struct btTreeNode *[childrenNum + 16];
            getline(infile, line);
            istringstream crenLi;
            crenLi.str(line);
            for (int j = 0; j < childrenNum; j++)
            {
                int child_number;
                crenLi >> child_number;
                struct btTreeNode *newNode = new struct btTreeNode;
                btnode->children[j] = newNode;
                btnodes[child_number] = newNode;
            }
        }
        else
        {
            preli_scale++;
        }
        if (i > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    infile.close();
    cout << "Step 4.1: Finished" << endl;
}

int s4_cmp(string s1, string s2)
{
    // 对两个字符串进行比较
    return s1.compare(s2) < 0;
}

void s4_spaceInit(string canfname)
{
    // 扫描空间初始化
    // 完善回溯树叶子结点的扫描空间信息，构造预扫描结点数组
    cout << "Step 4.2: Space Initialization" << endl;
    ifstream infile;
    string line;
    infile.open(canfname);
    // 跳过模板部分
    for (int i = 0; i < 7; i++)
    {
        getline(infile, line);
    }
    // 从这里开始即是正式的叶子结点信息
    // 每读取到一个叶子结点的全部信息，提取其表达式、字符优先级、归约地址，然后保存到
    // 回溯树叶子结点的字符优先级数组中，以及初始化扫描空间，并将该结点保存到预扫描结
    // 点数组中
    preli = new struct btTreeNode *[preli_scale + 16];
    int preli_tip = 0;
    int tick = 1;
    for (int i = 1; i <= preli_scale; i++)
    {
        int number;
        int level;
        getline(infile, line);
        sscanf(line.c_str(), "Node: %d, Level: %d", &number, &level);
        int parent_number;
        getline(infile, line);
        sscanf(line.c_str(), "Parent: %d", &parent_number);
        int inf;
        int sup;
        getline(infile, line);
        sscanf(line.c_str(), "Scale: <%d, %d>", &inf, &sup);
        int addrNum = sup - inf + 1;
        string e, t;
        int charLev[32];
        getline(infile, line);
        istringstream comLi;
        comLi.str(line);
        comLi >> t;
        comLi >> e;
        comLi >> t;
        for (int j = 0; j < 32; j++)
        {
            int clev;
            comLi >> clev;
            // charLev[j] = clev >= 2? level - clev + 2 : level;
            charLev[j] = level - clev + 2;
            // 改成这个代码之后，最大的字符优先级会比level多1，例如一个例子里面：
            // Node: 27705, Level: 15
            // Parent: 27660
            // Scale: <79058, 79070>
            // Expression: 2a0104880042100050ed8520fffa**** =>
            // 16 15 15 15 15 14 13 12 11 13 10 10 10 11 13 11 9
            // 8 8 8 8 7 6 5 4 4 3 2 1 1 1 1
            // 经过转换，最大的字符优先级就是16
            // 另外这里可能有一种特殊情况，那就是字符优先级中只有1没有2，换句话说，
            // 就是叶子结点只有一个地址，或者降维的时候没有找到相似维度
        }
        getline(infile, line);
        string addr[___AL_LEAFSCALE]; // 保存叶子结点信息下面的所有归约地址，下标从0开始
        for (int j = 0; j < addrNum; j++)
        {
            getline(infile, line);
            addr[j] = line;
        }
        // 已从一个叶子结点信息中提取出了所有数据，接下来要根据这些数据更新回溯树叶子结点和
        // 预扫描结点指针数组
        struct btTreeNode *btnode = btnodes[number];
        preli[preli_tip++] = btnode;
        for (int j = 0; j < 32; j++)
        {
            btnode->space.charLev[j] = charLev[j];
        }
        btnode->ratio = 0.0;
        string se = "";
        int dimension = 0;
        for (int j = 0; j < 32; j++)
        {
            if (btnode->space.charLev[j] >= level)
            {
                dimension++;
                se += e[j];
                e[j] = '-';
            }
        }
        btnode->space.dimension = dimension;
        btnode->space.expression = e;
        btnode->space.sub.card = 0;
        btnode->space.sub.num = 0;
        btnode->space.sub.elements = NULL;
        string base = se;
        int se_len = (int )(se.size()); // se.size() == dimension
        for (int j = 0; j < addrNum; j++)
        {
            int tip = 0;
            string f = addr[j];
            for (int k = 0; k < se_len; k++)
            {
                se[k] = se[k] == '*'? f[tip++] : se[k];
            }
            // 根据字符优先级数组，来将level最高的最后一个位置上的字符置为'*'
            int tt = 0;
            for (int k = 31; k >= 0; k--)
            {
                if (charLev[k] == level + 1)
                {
                    tt++;
                    break;
                }
                else if (charLev[k] == level)
                {
                    tt++;
                }
            }
            se[dimension - tt] = '*';
            addr[j] = se;
            se = base;
        }
        // 这一步结束之后addr[]等于expression[]/'*'，不过addr[]还没有排序和去重
        // 注意这里可能出现一个问题，那就是如果出现了叶子结点下面只有一个地址，并且初始
        // 扫描空间维度等于1，这样的话代码运行会出错，但推导了一下，当___AL_LEAFSCALE
        // 大于等于16时应该不会出现这个情况
        
        // 排序和去重
        sort(addr, addr + addrNum, s4_cmp);
        int naddrNum;
        naddrNum = (int )(unique(addr, addr + addrNum) - addr);
        // naddrNum即是card，将addr的信息放入expression和dgreef中
        btnode->space.add.card = naddrNum;
        btnode->space.add.num = naddrNum * 16;
        btnode->space.add.elements = new struct scanSet [naddrNum];
        struct scanSet *elements = btnode->space.add.elements;
        for (int j = 0; j < naddrNum; j++)
        {
            elements[j].expression = addr[j];
            elements[j].dgreef = 1;
        }
        btnode->status = ___AL_STA_READY;
        if (i > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
        
        // 这里有一个值得注意的地方，论文中没有将划分限界明确为base，也就是说16进制下也可以是8，
        // 这样的话可能会出现叶子节点一生成地址子空间就要回溯到父结点的情况，那样这段代码就要改，
        // 在将___AL_LEAFSCALE设置为大于等于16时不会出现这种情况，这样代码是没问题的。
        // => 后来在论文中明确了___AL_LEAFSCALE是等于BASE的
    }
    infile.close();
    cout << "Step 4.2: Finished" << endl;
}

void s4_entrance()
{
    cout << "Step 4: Backtracking Algorithm" << endl;
    s4_btTreeGenerate(string(___FL_TREE));
    s4_spaceInit(string(___FL_CANIPv6));
    cout << "Step 4: Finished" << endl;
}

// 第五步开始使用的类与函数

#if ___ENV == ___ENV_TEST
int s5_recSearch(string addr, struct seTreeNode *sit, int addr_size)
{
    // 采用递归的方法在查询树中搜索扫描空间下的活跃地址数量
    if (addr[0] == '*')
    {
        // 探测addr后面是否有非'*'的字符，如果没有的话，那么可以直接返回sit的数量，
        // 否则应该继续递归
        bool pure = true;
        for (int i = 1; i < addr_size; i++)
        {
            if (addr[i] != '*')
            {
                pure = false;
            }
        }
        if (pure == true)
        {
            return sit->ipnum;
        }
        else
        {
            int sum = 0;
            for (int i = 0; i < 16; i++)
            {
                if (sit->children[i] != NULL)
                {
                    sum += s5_recSearch(addr.substr(1), sit->children[i], addr_size - 1);
                }
            }
            return sum;
        }
    }
    else
    {
        int lo = addr[0] >= 'a' ? addr[0] - 'a' + 10 : addr[0] - '0';
        if (addr_size == 1 && sit->children[lo] != NULL)
        {
            return 1;
        }
        if (sit->children[lo] == NULL)
        {
            return 0;
        }
        else
        {
            return s5_recSearch(addr.substr(1), sit->children[lo], addr_size - 1);
        }
    }
}
#endif

#if ___ENV == ___ENV_TEST
int s5_recIpNum(struct seTreeNode *sit)
{
    // 采用递归的方法，计算出查询树中所有结点的ipnum值
    if (sit->leaf == true)
    {
        sit->ipnum = 1;
        return 1;
    }
    else
    {
        int sum = 0;
        for (int i = 0; i < 16; i++)
        {
            if (sit->children[i] != NULL)
            {
                sum += s5_recIpNum(sit->children[i]);
            }
        }
        sit->ipnum = sum;
        return sum;
    }
}
#endif

#if ___ENV == ___ENV_TEST
void s5_seTreeGeneration()
{
    // 根据test文件，生成查询树
    // 1. 初始化查询树结构的起始结点
    seXree.ipnum = 0;
    seXree.level = 0;
    seXree.ch = '-';
    seXree.leaf = false;
    seXree.parent = NULL;
    seXree.children[0] = NULL;
    seXree.children[1] = NULL;
    seXree.children[2] = NULL;
    seXree.children[3] = NULL;
    seXree.children[4] = NULL;
    seXree.children[5] = NULL;
    seXree.children[6] = NULL;
    seXree.children[7] = NULL;
    seXree.children[8] = NULL;
    seXree.children[9] = NULL;
    seXree.children[10] = NULL;
    seXree.children[11] = NULL;
    seXree.children[12] = NULL;
    seXree.children[13] = NULL;
    seXree.children[14] = NULL;
    seXree.children[15] = NULL;
    
    // 2. 从test文件中依次读取地址，然后扩展树结构
    string line;
    struct seTreeNode *start = &seXree;
    int num = 0;
    int tick = 1;
    while (getline(test, line))
    {
        // 此时的line即是一个16进制字符表示的IPv6地址，遍历它的32个
        // 字符，如果在查询树中没有，就添加结点
        struct seTreeNode *sit = start;
        for (int i = 0; i < 32; i++)
        {
            char ch = line[i];
            int lo = ch >= 'a' ? ch - 'a' + 10 : ch - '0';
            if (sit->children[lo] == NULL)
            {
                struct seTreeNode *newNode = new struct seTreeNode;
                newNode->ipnum = 0;
                newNode->level = sit->level + 1;
                newNode->ch = ch;
                newNode->leaf = false;
                newNode->parent = sit;
                newNode->children[0] = NULL;
                newNode->children[1] = NULL;
                newNode->children[2] = NULL;
                newNode->children[3] = NULL;
                newNode->children[4] = NULL;
                newNode->children[5] = NULL;
                newNode->children[6] = NULL;
                newNode->children[7] = NULL;
                newNode->children[8] = NULL;
                newNode->children[9] = NULL;
                newNode->children[10] = NULL;
                newNode->children[11] = NULL;
                newNode->children[12] = NULL;
                newNode->children[13] = NULL;
                newNode->children[14] = NULL;
                newNode->children[15] = NULL;
                sit->children[lo] = newNode;
            }
            sit = sit->children[lo];
        }
        sit->leaf = true;
        sit->leaf_addr = line;
        num++;
        if (num > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    
    // 3. 以递归的方式计算出除叶子结点之外所有结点的ipnum
    start->ipnum = s5_recIpNum(start);
}
#endif

void s5_scanPreface(string ffilename, string lfilename, string alfilename, string tfilename)
{
    // 扫描工作的前序步骤
    results.open(ffilename);
    alias.open(alfilename);
#if ___ENV == ___ENV_TEST
    test.open(tfilename);
    cout << "Experiment Type: Simulation" << endl;
    cout << "Preface: Search Tree Generation" << endl;
    s5_seTreeGeneration();
    cout << "Preface: Finished" << endl;
#elif ___ENV == ___ENV_WORK
    list.open(lfilename);
    cout << "Experiment Type: Actuality" << endl;
    srand((unsigned int)time(NULL));
#endif
    tfnum = 0;
    tsnum = 0;
}

string s5_tranStr(string expression, int prefix)
{
    // 生成标准的IPv6子网表达式
    string result = "";
    for (int i = 0; i < 8; i++)
    {
        string ts = expression.substr(i * 4, 4);
        string se = "";
        for (int j = 0; j < 4; j++)
        {
            if (ts[j] != '0')
            {
                se += ts[j];
            }
        }
        se += se.size() == 0? "0:" : ":";
        result += se;
    }
    int len = (int )result.size();
    result[len - 1] = '/';
    result += to_string(prefix);
    return result;
}

#if ___ENV == ___ENV_WORK
string s5_tranBiStr(string addr)
{
    // 将完全16进制表示的字符串（例如：593800740a5300000000000000000000）转换为完全
    // 2进制表示的字符串
    string tranLi[16] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110",
        "0111", "1000", "1001", "1010", "1011", "1100", "1101",
        "1110", "1111"};
    string res = "";
    for (int i = 0; i < 32; i++)
    {
        char ch = addr[i];
        if (ch >= '0' && ch <= '9')
        {
            int sit = ch - '0';
            res += tranLi[sit];
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            int sit = ch - 'a';
            res += tranLi[10 + sit];
        }
        else // ch >= 'A' && ch <= 'F'
        {
            int sit = ch - 'A';
            res += tranLi[10 + sit];
        }
    }
    return res;
}
#endif

#if ___ENV == ___ENV_WORK
int s5_snCmp(struct snTreeNode *node1, struct snTreeNode *node2)
{
    return node1->ip_addr.compare(node2->ip_addr) < 0;
}
#endif

void s5_ipv6Normalize(string &s1, string &s2)
{
    // 进行字符串分割
    vector<string> sg = s1_split(s1, ':');
    // 对分割后的字符串数组进行处理，首先得出数组的个数，
    // 当遇到空字符串时，进行零位补齐
    int num = (int )(sg.size());
    s2 = "";
    string zero1[7] = {"0000", "00000000", "000000000000", "0000000000000000",
        "00000000000000000000", "000000000000000000000000", "0000000000000000000000000000"};
    string zero2[4] ={"", "0", "00", "000"};
    // 可能出现IPv6+IPv4表示的特殊情况
    // 例如：2001:250:7003:a:200:5efe:210.30.184.209
    // 首先判断最后一个字符串中是否有'.'字符，如果有，则将flag执为1
    int flag = sg[num - 1].find('.') == string::npos? 0 : 1;
    for (int i = 0; i < num; ++i)
    {
        if (sg[i].size() == 0)
        {
            s2 += zero1[8 - num - flag];
        }
        else
        {
            // 判断是否是IPv4表示，如果是IPv4，则转换为8个16进制字符
            // 如果不是IPv4，那么计算字符串中数字和字母的个数，如果不足4个那么在前面补0
            string k = sg[i];
            if (k.find('.') != string::npos)
            {
                // 将k按照'.'进行切分，然后依次转换为16进制字符
                vector<string> s4 = s1_split(k, '.');
                int snum = (int )(s4.size());
                for (int j = 0; j < snum; ++j)
                {
                    // 将其中的十进制字符串转换为16进制字符串
                    string t = s1_DecStrToHexStr(s4[j]);
                    s2 += t.size()==2? t : ("0" + t);
                }
            }
            else
            {
                int cnum = k[k.size() - 1]=='\n'? (int )(k.size()) - 1:(int )(k.size());
                s2 += zero2[4 - cnum] + k;
            }
        }
    }
}

bool s5_belong(string aip, struct snTreeNode *snnode)
{
    // 当aip属于这个子网树结点时，返回true，否则返回false
    // 目前aip是完全16进制格式表示的，二者都可以有自由符
    string ip_addr = snnode->ip_addr;
    for (int i = 0; i < 32; i++)
    {
        if (aip[i] != ip_addr[i] && ip_addr[i] != '*')
        {
            return false;
        }
    }
    return true;
}

#if ___ENV == ___ENV_WORK
string s5_tranHexStr(string addr)
{
    // 将完全2进制表示的字符串转换为完全16进制表示的字符串
    string tranLi[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8",
        "9", "a", "b", "c", "d", "e", "f"
    };
    string res = "";
    for (int i = 0; i < 32; i++)
    {
        int ch1 = addr[i * 4 + 0] - '0';
        int ch2 = addr[i * 4 + 1] - '0';
        int ch3 = addr[i * 4 + 2] - '0';
        int ch4 = addr[i * 4 + 3] - '0';
        int sit = ch1 * 8 + ch2 * 4 + ch3 * 2 + ch4;
        res += tranLi[sit];
    }
    return res;
}
#endif

#if ___ENV == ___ENV_WORK
void s5_inputAddr(string addr, int prefix)
{
    // 提取子网链表结点中的信息，并将其中的所有地址以完全16进制字符串的格式写入到数组中
    // 如果prefix加上add已经等于128，那么直接将addr写入到targetArr数组中
    if (prefix == 128)
    {
        targetArr[targetArr_tip++] = addr;
        return ;
    }
    addr[prefix / 4] = '0';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '1';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '2';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '3';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '4';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '5';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '6';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '7';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '8';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = '9';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'a';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'b';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'c';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'd';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'e';
    s5_inputAddr(addr, prefix + 4);
    addr[prefix / 4] = 'f';
    s5_inputAddr(addr, prefix + 4);
}
#endif

#if ___ENV == ___ENV_WORK
string s5_tranIPv6(string addr)
{
    // 将地址从完全16进制字符串格式转换为标准IPv6格式
    string res = "";
    for (int i = 0; i < 7; i++)
    {
        string sp = addr.substr(i * 4, 4);
        int j = 0;
        for (; j < 4; j++)
        {
            if (sp[j] != '0')
            {
                break;
            }
        }
        if (j == 4)
        {
            res += '0';
        }
        for (; j < 4; j++)
        {
            res += sp[j];
        }
        res += ':';
    }
    string sp = addr.substr(28, 4);
    int j = 0;
    for (; j < 4; j++)
    {
        if (sp[j] != '0')
        {
            break;
        }
    }
    if (j == 4)
    {
        res += '0';
    }
    for (; j < 4; j++)
    {
        res += sp[j];
    }
    return res;
}
#endif

void s5_recReleaseSnTree(struct snTreeNode *snnode)
{
    // 递归释放子网树结构
    if (snnode == NULL)
    {
        return ;
    }
    if (snnode->leaf == false)
    {
        for (int i = 0; i < 16; i++)
        {
            s5_recReleaseSnTree(snnode->children[i]);
        }
    }
    delete snnode;
}

#if ___ENV == ___ENV_WORK
void s5_recInputLeafAddr(string ip_addr, int start)
{
    // 根据地址表达式将对应的所有地址写入到targetArr数组中
    int sit;
    for (sit = start; sit < 32; sit++)
    {
        if (ip_addr[sit] == '*')
        {
            break;
        }
    }
    if (sit == 32)
    {
        targetArr[targetArr_tip++] = ip_addr;
        return ;
    }
    ip_addr[sit] = '0';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '1';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '2';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '3';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '4';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '5';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '6';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '7';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '8';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = '9';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'a';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'b';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'c';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'd';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'e';
    s5_recInputLeafAddr(ip_addr, sit + 1);
    ip_addr[sit] = 'f';
    s5_recInputLeafAddr(ip_addr, sit + 1);
}
#endif

#if ___ENV == ___ENV_WORK
void s5_recInputAddr(struct snTreeNode *snnode)
{
    // 递归遍历子网树结构，将地址写入到targetArr数组中
    if (snnode->leaf == true)
    {
        s5_recInputLeafAddr(snnode->ip_addr, 0);
        return ;
    }
    for (int i = 0; i < 16; i++) // snnode->leaf == false
    {
        if (snnode->children[i] != NULL)
        {
            s5_recInputAddr(snnode->children[i]);
        }
    }
}
#endif

void s5_fuseExpr(string &expression, string sexpression, string texpression)
{
    int sexpression_tip = 0;
    for (int i = 0; i < 32; i++)
    {
        if (texpression[i] == '-')
        {
            expression += sexpression[sexpression_tip++];
        }
        else
        {
            expression += texpression[i];
        }
    }
}

void s5_leavesScan(int left, int right, long objects_scale)
{
    // 对preli数组中[left, right]范围内的叶子结点执行扫描，并保存结果到收获文件中，同时
    // 更新其中每个叶子结点的ratio，这里sum等于要扫描的地址数量
#if ___ENV == ___ENV_TEST
    // 进行模拟扫描，此时不需调用objects数组存放IPv6地址并随机化顺序，借助每个
    // 结点下的扫描空间加集族信息，再借助查询树，即可快速得到扫描结果，每次执行了扫描，就将
    // 扫描空间信息以及发现的活跃IPv6地址数量保存到发现结果文件中
    struct btTreeNode **arr = preli;
    long fnum = 0; // 本次扫描中累计发现的地址个数
    for (int i = left; i <= right; i++)
    {
        struct btTreeNode *leaf = arr[i];
        
        int card = leaf->space.add.card;
        struct scanSet *elements = leaf->space.add.elements;
        string expression = leaf->space.expression;
        int lfnum = 0; // 特定叶子结点下发现的地址个数
        for (int j = 0; j < card; j++)
        {
            string sspace = elements[j].expression;
            int tip = 0;
            string cspace = "";
            for (int k = 0; k < 32; k++)
            {
                if (expression[k] == '-')
                {
                    cspace += sspace[tip++];
                }
                else
                {
                    cspace += expression[k];
                }
            }
            // results << cspace << endl;
            // cspace即是完整的可以用于查询树的扫描空间表示式，可能出现56**7这样的格式，所以下面的查询树
            // 算法应该用递归的方法来实现
            int rnum = s5_recSearch(cspace, &seXree, 32);
            lfnum += rnum;
            fnum += (long )rnum;
        }
        leaf->status = ___AL_STA_DONE;
        leaf->fnum = (long )lfnum;
        leaf->ratio = (double )(lfnum) / (double )(leaf->space.add.num);
    }
    tfnum += fnum;
    tsnum += objects_scale;
    results << "      --Leaf Scanning--" << endl;
    results << "Active Number: " << fnum << endl;
    results << "Scanning Number: " << objects_scale << endl;
    results << "Total Active Number: " << tfnum << endl;
    results << "Total Scanning Number: " << tsnum << endl;
    //cout << "      --Leaf Scanning--" << endl;
    //cout << "Active Number: " << fnum << endl;
    //cout << "Scanning Number: " << objects_scale << endl;
    //cout << "Total Active Number: " << tfnum << endl;
    //cout << "Total Scanning Number: " << tsnum << endl;
#elif ___ENV == ___ENV_WORK
    // 进行实际扫描，遍历[left, right]范围内的结点，对每个结点，将它的待扫描空间提取出来，
    // 保存到一个临时文档中，然后调用ZMapv6扫描，并将活跃地址数量统计出来
    
    // 1. 依次遍历[left, right]的每个结点，每遍历到一个结点，根据其加集族信息生成子网森林，
    //    然后将其中所有地址保存到targetArr中，这样遍历完所有结点
    struct btTreeNode **arr = preli;
    targetArr_scale = objects_scale;
    targetArr_tip = 0;
    targetArr = new string [targetArr_scale + 16];
    snXree_scale = 0;
    for (int i = left; i <= right; i++)
    {
        
        struct btTreeNode *leaf = arr[i];
        snXree_scale += leaf->space.add.card;
    }
    snXree = new struct snTreeNode *[snXree_scale + 16];
    snXree_tip = 0;
    for (int i = left; i <= right; i++)
    {
        
        struct btTreeNode *leaf = arr[i];
        leaf->ratio = 0.0;
        leaf->fnum = 0;
        string texpression = leaf->space.expression;
        // 将该结点的加集族信息保存到子网森林中
        int acard = leaf->space.add.card;
        for (int j = 0; j < acard; j++)
        {
            string sexpression = leaf->space.add.elements[j].expression;
            string expression = "";
            // 根据texpression和sexpression生成初步的子网表达式
            s5_fuseExpr(expression, sexpression, texpression);
            // expression即是初步的子网表达式，它的形式可能是56*78**这样的字符串，
            // 字符串中部也可能会有自由度符号，要根据它生成自由符仅在尾部的子网表达式，
            // 并用子网树的形式表示出来
            snXree[snXree_tip] = new struct snTreeNode;
            struct snTreeNode *snnode = snXree[snXree_tip];
            snXree_tip++;
            snnode->ip_addr = expression;
            snnode->dgreef = leaf->space.add.elements[j].dgreef;
            snnode->btnode_number = leaf->number;
            snnode->leaf = true;
            snnode->parent = NULL;
        }
    }
    
    // 2. 从子网森林写入地址到targetArr，然后将其中的所有地址随机化后写入到target文件中
    for (int i = 0; i < snXree_scale; i++)
    {
        struct snTreeNode *snnode = snXree[i];
        s5_recInputAddr(snnode);
    }
    s5_RandomGenerator sg;
    //random_shuffle(targetArr, targetArr + objects_scale, sg);
    // 将targetArr数组中的元素转换为标准IPv6表达格式，然后保存到target文件中
    ofstream targetfile;
    targetfile.open(___FL_TARGET);
    for (long i = 0; i < objects_scale; i++)
    {
        targetfile << s5_tranIPv6(targetArr[i]) << endl;
    }
    targetfile.close();
    
    /*// 3. 调用ZMapv6进行扫描
    string ins = string(___PR_ZMAP) + " --probe-module=icmp6_echoscan --ipv6-target-file=" + string(___FL_TARGET);
    ins += " --ipv6-source-ip=" + string(___PR_SOURCE) + " --bandwidth=" + string(___PR_BAND);
    ins += " --output-file=" + string(___FL_ZRES) + " --cooldown-time=" + string(___PR_CTIME);
    system(ins.c_str());
    */
    // 3. Use Icmpv6
    string ins = "python2 ping6.py " + string(___FL_TARGET) +" "+ string(___FL_ZRES);
    system(ins.c_str());

    // 4. 统计扫描情况，更新各个结点的信息，包括其ratio值和status值
    string *active; // 存放活跃IPv6地址的数组，下标从0开始，以完全16进制格式表示
    int active_scale = 0; // 活跃IPv6地址的数量
    ifstream resfile;
    resfile.open(___FL_ZRES);
    string line;
    while (getline(resfile, line))
    {
        active_scale++;
    }
    resfile.close();
    active = new string [active_scale + 16];
    active_scale = 0;
    resfile.open(___FL_ZRES);
    while (getline(resfile, line))
    {
        string ipv6Nm = "";
        s5_ipv6Normalize(line, ipv6Nm);
        active[active_scale++] = ipv6Nm;
    }
    resfile.close();
    sort(active, active + active_scale, s4_cmp);
    sort(snXree, snXree + snXree_scale, s5_snCmp);
    snXree_tip = 0;
    for (long i = 0; i < active_scale; i++)
    {
        string aip = active[i];
        for (; snXree_tip < snXree_scale; snXree_tip++)
        {
            if (s5_belong(aip, snXree[snXree_tip]) == true)
            {
                int btnode_number = snXree[snXree_tip]->btnode_number;
                btnodes[btnode_number]->fnum += 1;
                break;
            }
        }
    }
    for (int i = left; i <= right; i++)
    {
        struct btTreeNode *leaf = arr[i];
        int num = leaf->space.add.num;
        long fn = leaf->fnum;
        leaf->ratio = ((double )fn) / (double )num;
        leaf->status = ___AL_STA_DONE;
    }
    
    // 5. 输出统计结果
    tfnum += active_scale;
    tsnum += objects_scale;
    results << "      --Leaf Scanning--" << endl;
    results << "Active Number: " << active_scale << endl;
    results << "Scanning Number: " << objects_scale << endl;
    results << "Total Active Number: " << tfnum << endl;
    results << "Total Scanning Number: " << tsnum << endl;
    for (int i = 0; i < active_scale; i++)
    {
        list << active[i] << endl;
    }
    cout << "      --Leaf Scanning--" << endl;
    cout << "Active Number: " << active_scale << endl;
    cout << "Scanning Number: " << objects_scale << endl;
    cout << "Total Active Number: " << tfnum << endl;
    cout << "Total Scanning Number: " << tsnum << endl;
    
    // 6. 释放动态数据
    delete [] active;
    for (int i = 0; i < snXree_scale; i++)
    {
        s5_recReleaseSnTree(snXree[i]);
    }
    delete [] snXree;
    delete [] targetArr;
#endif
}

void s5_preScan()
{
    // 根据preli数组，开始执行预扫描，预扫描的步骤如下：
    // 遍历preli数组，采用“逐次积累、过阈触发”的方式，将叶子结点成堆地送至扫描节点进行
    // 扫描，扫描时将叶子结点下面表示的所有地址取出来，放入objects数组中，然后进行序列
    // 随机化，之后开始并行扫描，一旦发现目标是活跃的，就将目标保存到收获文件results中
    // ，并将目标bundle所在的编号对应的叶子结点的ratio值加1.0，在扫描完毕之后将这些
    // 叶子结点的ratio除以其扫描空间的地址数量，即加集族的num值减去减集族的num值
    cout << "Step 5.1: Leaves Scanning" << endl;
#if ___ENV == ___ENV_TEST
    results << "Experiment Type: Simulation" << endl;
#elif ___ENV == ___ENV_WORK
    results << "Experiment Type: Actuality" << endl;
#endif
    results << "Pre-scanning:" << endl;
    int left;
    int right;
    long sum; // 累积的待扫描地址数量，用于判断是否要将数据送入扫描节点
    struct btTreeNode **arr = preli;
    int scale = preli_scale;
    left = 0;
    sum = 0;
    for (right = 0; right < scale; right++)
    {
        int num = arr[right]->space.add.num; // 预扫描不需要减去减集族地址数量，因为为0
        // 所有预扫描节点都处于READY状态
        sum += (long )num;
        if (sum > ___AL_SCAN_THD)
        {
            s5_leavesScan(left, right, sum);
            sum = 0;
            left = right + 1;
        }
    }
    if (left != scale)
    {
        s5_leavesScan(left, scale - 1, sum);
    }
    cout << "Step 5.1: Finished" << endl;
}

int s5_cmp(struct btTreeNode *node1, struct btTreeNode *node2)
{
    // 对两个叶子结点的ratio值进行比较
    return node1->ratio - node2->ratio > 0.0;
}

void s5_sortLink()
{
    // 根据扫描结果，将叶子结点排序
    cout << "Step 5.2: Results Sorting and Linked List Generation" << endl;
    // 此时preli中的所有叶子结点都处于DONE状态，对preli数组根据其ratio值进行排序，然后生成链表
    sort(preli, preli+preli_scale, s5_cmp);
    liXree = new struct LinkNode;
    liXree->node = preli[0];
    liXree->next = NULL;
    struct LinkNode *sit = liXree;
    for (int i = 1; i < preli_scale; i++)
    {
        struct LinkNode *t = new struct LinkNode;
        t->node = preli[i];
        t->next = NULL;
        sit->next = t;
        sit = t;
    }
    cout << "Step 5.2: Finished" << endl;
}

void s5_releasePreLi()
{
    // 释放预扫描结点指针数组
    delete [] preli;
}

void s5_entrance()
{
    cout << "Step 5: Pre-scanning" << endl;
    s5_scanPreface(string(___FL_FIND), string(___FL_LIST), string(___FL_ALIAS), string(___FL_TEST));
    s5_preScan();//only extend one bit
    s5_sortLink();
    s5_releasePreLi();
    cout << "Step 5: Finished" << endl;
}

// 第六步开始使用的函数

#if ___ENV == ___ENV_TEST //configure to work
void s6_recFree(struct seTreeNode *sit)
{
    // 用递归的方式释放查询树
    if (sit->leaf == false)
    {
        for (int i = 0; i < 16; i++)
        {
            if (sit->children[i] != NULL)
            {
                s6_recFree(sit->children[i]);
            }
        }
    }
    if (sit->parent != NULL)
    {
        delete sit;
    }
}
#endif

long s6_recunpnFnum(struct btTreeNode *sit)
{
    // 在开始之前还要将回溯树中的所有UNOPENED状态结点的fnum计算出来
    // 将所有UNOPENED结点的fnum计算出来之后，将其置为FINISH状态
    if (sit->status == ___AL_STA_UNOPED)
    {
        long fnum = 0;
        for (int i = 0; i < sit->childrenNum; i++)
        {
            if (sit->children[i]->status != ___AL_STA_UNOPED)
            {
                fnum += sit->children[i]->fnum;
            }
            else
            {
                fnum += s6_recunpnFnum(sit->children[i]);
            }
        }
        sit->fnum = fnum;
        sit->status = ___AL_STA_FINISH;
        return fnum;
    }
    else
    {
        return sit->fnum;
    }
}

void s6_storeTree()
{
    s6_recunpnFnum(&btXree);
    ofstream frut;
    frut.open(string(___FL_FRUT));
    for (int i = 1; i <= btnodes_scale; i++)
    {
        frut << "node " << i << endl;
        if (btnodes[i]->parent == NULL)
        {
            frut << "parent 0" << endl;
        }
        else
        {
            frut << "parent " << btnodes[i]->parent->number << endl;
        }
        frut << "fnum: " << btnodes[i]->fnum << endl;
        int l = 0;
        if (btnodes[i]->parent == NULL)
        {
            l = 128;
        }
        else
        {
            string exp = btnodes[i]->parent->space.expression;
            for (int i = 0; i < 32; i++)
            {
                if (exp[i] == '-')
                {
                    l += 4;
                }
            }
        }
        frut << "scale: " << l << endl;
        frut << "color: " << ((double )(l) - log2((double )(btnodes[i]->fnum))) << endl;
        frut << endl;
    }
    frut.close();
}

void s6_scanEpilogue()
{
    // 扫描工作的尾处理
    results.close();
    alias.close();
#if ___ENV == ___ENV_TEST
    cout << "Epilogue: Release Search Tree" << endl;
    test.close();
    s6_recFree(&seXree);
    cout << "Epilogue: Finished" << endl;
    cout << "Epilogue: Store Tree Information" << endl;
    s6_storeTree();
    cout << "Epilogue: Finished" << endl;
#elif ___ENV == ___ENV_WORK
    cout << "Epilogue: Store Tree Information" << endl;
    s6_storeTree();
    cout << "Epilogue: Finished" << endl;
    list.close();
#endif
}

void s6_releaseLink()
{
    // 释放链表
    struct LinkNode *sit = liXree;
    while (sit != NULL)
    {
        struct LinkNode *t = sit->next;
        delete sit;
        sit = t;
    }
}

void s6_releaseBtTree()
{
    // 释放回溯树结构及其结点数组
    for (int i = 2; i <= btnodes_scale; i++)
    {
        if (btnodes[i]->children != NULL)
        {
            delete [] btnodes[i]->children;
        }
        if (btnodes[i]->space.add.card != 0)
        {
            delete [] btnodes[i]->space.add.elements;
        }
        if (btnodes[i]->space.sub.card != 0)
        {
            delete [] btnodes[i]->space.sub.elements;
        }
        delete btnodes[i];
    }
    delete [] btnodes[1]->children;
    if (btnodes[1]->space.add.card != 0)
    {
        delete [] btnodes[1]->space.add.elements;
    }
    if (btnodes[1]->space.sub.card != 0)
    {
        delete [] btnodes[1]->space.sub.elements;
    }
}

int s6_cmp(struct scanSet set1, struct scanSet set2)
{
    // 对两个扫描集合中的表达式字符串进行比较
    return set1.expression.compare(set2.expression) < 0;
}

int s6_unique(struct scanSet *elements, int card)
{
    // 对已经排好序的elements数组进行去重，数组长度为card，
    // 将去重后的新元素放到数组低地址，并且返回新元素个数
    int last = 0;
    int i = last + 1;
    while (i < card)
    {
        if (elements[i].expression != elements[last].expression)
        {
            struct scanSet tp = elements[i];
            elements[i] = elements[last + 1];
            elements[last + 1] = tp;
            last++;
        }
        i++;
    }
    return last + 1;
}

struct eLink *s6_eLinkEnd(struct eLink *start)
{
    // 返回eLink链表的末尾结点
    struct eLink *tp = start;
    while (tp->next != NULL)
    {
        tp = tp->next;
    }
    return tp;
}

int s6_speRelation(struct eLink *tp, struct eLink *tq)
{
    // 比较tp和tq指向的字符串表达式之间的空间关系
    string tps = tp->expression;
    string tqs = tq->expression;
    for (int i = 0; i < 32; i++)
    {
        if (tps[i] != tqs[i])
        {
            if (tps[i] == '*' && tqs[i] != '*')
            {
                return ___AL_SPE_INC;
            }
            else if (tps[i] != '*' && tqs[i] == '*')
            {
                return ___AL_SPE_INCD;
            }
            else if (tps[i] != '*' && tqs[i] != '*')
            {
                return ___AL_SPE_NOIN;
            }
        }
    }
    return ___AL_SPE_INCD; // 说明两个空间是相等的，结合代码运行要求，将其记为INCD
}

void s6_moveRedun(struct eLink *es)
{
    // 表达式链表冗余归并
    // 1. 获取链表结点个数n
    int len = 0;
    struct eLink *tp = es;
    while (tp != NULL)
    {
        len++;
        tp = tp->next;
    }
    
    // 2. 做一次n*(n+1)/2的遍历，将其中的元素进行两两比对，一旦有元素是被包含的，
    //    就将其表达式更改为“-”
    tp = es;
    for (int i = 0; i < len - 1; i++, tp = tp->next)
    {
        if (tp->expression != "-")
        {
            struct eLink *tq = tp->next;
            for (int j = i + 1; j < len; j++, tq = tq->next)
            {
                if (tq->expression != "-")
                {
                    int sta = s6_speRelation(tp, tq);
                    if (sta == ___AL_SPE_INC)
                    {
                        tq->expression = "-";
                    }
                    else if (sta == ___AL_SPE_INCD)
                    {
                        tp->expression = "-";
                        break;
                    }
                }
            }
        }
    }
    
    // 3. 去除链表中表达式是“-”的元素，结束
    // es指向的结点不能被删除，如果它的表达式是”-“，那么可以与后面的表达式不为“-”
    // 的结点互换表达式，整个链表中不可能全是“-”
    struct eLink *start = es;
    if (start->expression == "-")
    {
        struct eLink *next = start->next;
        while (next != NULL)
        {
            if (next->expression != "-")
            {
                break;
            }
            else
            {
                next = next->next;
            }
        }
        start->expression = next->expression;
        next->expression = "-";
    }
    struct eLink *last = es;
    struct eLink *sit = es->next;
    while (sit != NULL)
    {
        if (sit->expression == "-")
        {
            last->next = sit->next;
            delete sit;
            sit = last->next;
        }
        else
        {
            last = sit;
            sit = sit->next;
        }
    }
}

struct eLink *s6_recScannedSpe1(struct btTreeNode *parent, long &fnum)
{
    // 用递归的方法求出父结点及其以下的已扫描空间，并以表达式链表的形式返回
    // READY结点的取减集族空间，DONE结点的取加集族空间
    if (parent->status == ___AL_STA_DONE)
    {
        // 从它的加集族信息中提取出表达式，构成表达式链表并返回
        int card = parent->space.add.card;
        struct scanSet *elements = parent->space.add.elements;
        string texpression = parent->space.expression;
        struct eLink *es = NULL;
        for (int i = 0; i < card; i++)
        {
            string sexpression = elements[i].expression;
            string expression = "";
            s5_fuseExpr(expression, sexpression, texpression);
            struct eLink *tp = new struct eLink;
            tp->expression = expression;
            tp->next = es;
            es = tp;
        }
        fnum += parent->fnum;
        parent->status = ___AL_STA_FINISH;
        return es;
    }
    else if (parent->status == ___AL_STA_READY)
    {
        // 从它的减集族信息中提取出表达式，构成表达式链表并返回
        int card = parent->space.sub.card;
        struct scanSet *elements = parent->space.sub.elements;
        string texpression = parent->space.expression;
        struct eLink *es = NULL;
        for (int i = 0; i < card; i++)
        {
            string sexpression = elements[i].expression;
            string expression = "";
            s5_fuseExpr(expression, sexpression, texpression);
            struct eLink *tp = new struct eLink;
            tp->expression = expression;
            tp->next = es;
            es = tp;
        }
        parent->status = ___AL_STA_FINISH;
        fnum += parent->fnum;
        return es;
    }
    else if (parent->status == ___AL_STA_FINISH) // 这个情况按理说不会出现
    {
        return NULL;
    }
    else // parent->status == ___AL_STA_UNOPED
    {
        // 递归遍历其所有子结点，提取出表达式，构成表达式链表，然后返回
        int childrenNum = parent->childrenNum;
        struct btTreeNode **children = parent->children;
        struct eLink *es = NULL;
        long taddrNum = 0;
        for (int i = 0; i < childrenNum; i++)
        {
            long lfnum = 0;
            struct eLink *tp = s6_recScannedSpe1(children[i], lfnum);
            if (tp == NULL)
            {
                continue;
            }
            else
            {
                long addrNum = 0;
                struct eLink *q = tp;
                while (q != NULL)
                {
                    string spe = q->expression;
                    int si = 0;
                    for (int j = 0; j < 32; j++)
                    {
                        if (spe[j] == '*')
                        {
                            si++;
                        }
                    }
                    if (si * 4 > 30)
                    {
                        addrNum = (1 << 30);
                    }
                    else
                    {
                        addrNum += (1 << (si * 4));
                    }
                    q = q->next;
                }
                taddrNum += addrNum;
                struct eLink *tp_end = s6_eLinkEnd(tp);
                tp_end->next = es;
                es = tp;
            }
            fnum += lfnum;
        }
        parent->status = ___AL_STA_FINISH;
        return es;
    }
}

struct eLink *s6_recScannedSpe2(struct eLink *readySpe, struct btTreeNode *pa, int dimension, int dgreef, int level, long &fnum)
{
    // 遍历pa父结点下面的所有子结点，找出空间被覆盖了的子结点，然后对这些子结点用递归遍历的方法把已扫描空间提取出来
    int childrenNum = pa->childrenNum;
    struct btTreeNode **children = pa->children;
    struct eLink *res = NULL;
    double taaddr = 0; // 累计的已扫描空间下活跃地址数
    long taddr = 0; // 累计的已扫描空间下地址数
    for (int i = 0; i < childrenNum; i++)
    {
        struct btTreeNode *chid = children[i];
        // 与readySpe中的空间进行比较，如果被覆盖了就将结点下已扫描空间以链表形式返回（递归遍历找出来），
        // 并将该结点直接置为FINISH状态，否则不改变结点
        
        // 比较方法：遍历readySpe中的每个字符串s1，然后用s1和chid结点的空间进行比较，求出其前相似度l，如果
        // dimension - s1的dgreef小于等于l，那么chid结点就是被覆盖了，否则没有被覆盖
        string s2 = "";
        if (chid->status == ___AL_STA_DONE || chid->status == ___AL_STA_READY)
        {
            // 直接取出其局部表达式
            s2 = chid->space.add.elements[0].expression;
        }
        else if (chid->status == ___AL_STA_UNOPED)
        {
            // 根据其全局表达式和其父结点的全局表达式生成其局部表达式
            string o_exp = chid->space.expression;
            string p_exp = chid->parent->space.expression;
            for (int i = 0; i < 32; i++)
            {
                if (p_exp[i] == '-' && o_exp[i] != '-')
                {
                    s2 += o_exp[i];
                }
                else if (p_exp[i] == '-' && o_exp[i] == '-')
                {
                    s2 += '*';
                }
            }
        }
        else // chid->status == ___AL_STA_FINISH 的情况不用管
        {
            continue;
        }
        // 将sexpression和readySpe中的每个字符串s1比较
        bool ind = false;
        for (struct eLink *rp = readySpe; rp != NULL; rp = rp->next)
        {
            string s1 = rp->expression; // 要看s1是不是包含s2，如果包含了就返回true，否则返回false
            // -- below is old code
            //            int l = 0;
            //            while ((s1[l] == s2[l]) || (s1[l] == '*') || (s2[l] == '*'))
            //            {
            //                l++;
            //            }
            //            if (dimension - dgreef <= l)
            //            {
            //                ind = true;
            //                break;
            //            }
            // -- up is old code
            // -- below is new code
            // 默认认为是不包含的，只有完全确定被包含了才将ind更改为true
            int len = (int )s1.length();
            int k;
            for (k = 0; k < len; k++)
            {
                if (s2[k] == '*' && s1[k] != '*')
                {
                    break;
                }
                if (s2[k] != s1[k] && s2[k] != '*' && s1[k] != '*')
                {
                    break;
                }
            }
            if (k == len)
            {
                ind = true;
            }
            // -- up is new code
        }
        if (ind == true)
        {
            // 肯定是会有被包含的，因为被扩展的结点自己必定被包含，如果一直没有被包含的，那就有问题
            if (chid->status == ___AL_STA_DONE)
            {
                int card = chid->space.add.card;
                struct scanSet *elements = chid->space.add.elements;
                string texpression = chid->space.expression;
                struct eLink *es = NULL;
                for (int i = 0; i < card; i++)
                {
                    string sexpression = elements[i].expression;
                    string expression = "";
                    s5_fuseExpr(expression, sexpression, texpression);
                    struct eLink *tp = new struct eLink;
                    tp->expression = expression;
                    tp->next = es;
                    es = tp;
                }
                chid->status = ___AL_STA_FINISH;
                // 从es的地址里面计算出地址数量，并完成相应工作
                double lrat = chid->ratio;
                long laddr = 0;
                struct eLink *es_end = NULL;
                struct eLink *q = es;
                while (q->next != NULL)
                {
                    string spe = q->expression;
                    int si = 0;
                    for (int j = 0; j < 32; j++)
                    {
                        if (spe[j] == '*')
                        {
                            si++;
                        }
                    }
                    if (si * 4 > 30)
                    {
                        laddr = (1 << 30);
                    }
                    else
                    {
                        laddr += (1 << (si * 4));
                    }
                    q = q->next;
                }
                string spe = q->expression;
                int si = 0;
                for (int j = 0; j < 32; j++)
                {
                    if (spe[j] == '*')
                    {
                        si++;
                    }
                }
                if (si * 4 > 30)
                {
                    laddr = (1 << 30);
                }
                else
                {
                    laddr += (1 << (si * 4));
                }
                taaddr += lrat * laddr;
                taddr += laddr;
                es_end = q;
                es_end->next = res;
                res = es;
            }
            else if (chid->status == ___AL_STA_READY)
            {
                int card = chid->space.sub.card;
                struct scanSet *elements = chid->space.sub.elements;
                string texpression = chid->space.expression;
                struct eLink *es = NULL;
                for (int i = 0; i < card; i++)
                {
                    string sexpression = elements[i].expression;
                    string expression = "";
                    s5_fuseExpr(expression, sexpression, texpression);
                    struct eLink *tp = new struct eLink;
                    tp->expression = expression;
                    tp->next = es;
                    es = tp;
                }
                chid->status = ___AL_STA_FINISH;
                // 从es的地址里面计算出地址数量，并完成相应工作
                double lrat = chid->ratio; // 此时结点处于READY状态，ratio保存的应该是减集族空间的AAP
                long laddr = 0;
                struct eLink *es_end = NULL;
                struct eLink *q = es;
                while (q->next != NULL)
                {
                    string spe = q->expression;
                    int si = 0;
                    for (int j = 0; j < 32; j++)
                    {
                        if (spe[j] == '*')
                        {
                            si++;
                        }
                    }
                    if (si * 4 > 30)
                    {
                        laddr = (1 << 30);
                    }
                    else
                    {
                        laddr += (1 << (si * 4));
                    }
                    q = q->next;
                }
                string spe = q->expression;
                int si = 0;
                for (int j = 0; j < 32; j++)
                {
                    if (spe[j] == '*')
                    {
                        si++;
                    }
                }
                if (si * 4 > 30)
                {
                    laddr = (1 << 30);
                }
                else
                {
                    laddr += (1 << (si * 4));
                }
                taaddr += lrat * laddr;
                taddr += laddr;
                es_end = q;
                es_end->next = res;
                res = es;
            }
            else // chid->status == ___AL_STA_UNOPEND
            {
                // 从es的地址里面计算出地址数量，并完成相应工作
                long lfnum = 0;
                struct eLink *es = s6_recScannedSpe1(chid, lfnum);
                chid->status = ___AL_STA_FINISH;
                long laddr = 0;
                struct eLink *es_end = NULL;
                struct eLink *q = es;
                while (q->next != NULL)
                {
                    string spe = q->expression;
                    int si = 0;
                    for (int j = 0; j < 32; j++)
                    {
                        if (spe[j] == '*')
                        {
                            si++;
                        }
                    }
                    if (si * 4 > 30)
                    {
                        laddr = (1 << 30);
                    }
                    else
                    {
                        laddr += (1 << (si * 4));
                    }
                    q = q->next;
                }
                string spe = q->expression;
                int si = 0;
                for (int j = 0; j < 32; j++)
                {
                    if (spe[j] == '*')
                    {
                        si++;
                    }
                }
                if (si * 4 > 30)
                {
                    laddr = (1 << 30);
                }
                else
                {
                    laddr += (1 << (si * 4));
                }
                taaddr += lfnum;
                taddr += laddr;
                es_end = q;
                es_end->next = res;
                res = es;
            }
        }
    }
    fnum += taaddr;
    return res;
}

int s6_getSexpre(string &sexpression, string parent_expression, string expression)
{
    // 从父结点空间表达式和完全表达式推导出扫描集合表达式，并返回*的数量
    int index = 0;
    for (int i = 0; i < 32; i++)
    {
        if (parent_expression[i] == '-')
        {
            sexpression += expression[i];
            if (expression[i] == '*')
            {
                index++;
            }
        }
    }
    return index;
}

void s6_recFinish(struct btTreeNode *parent)
{
    // 将该结点以及下面的所有结点置为FINISH状态
    parent->status = ___AL_STA_FINISH;
    int childrenNum = parent->childrenNum;
    for (int i = 0; i < childrenNum; i++)
    {
        s6_recFinish(parent->children[i]);
    }
}

void s6_releaseELink(struct eLink *link)
{
    struct eLink *tp = link;
    while (tp != NULL)
    {
        struct eLink *tt = tp;
        tp = tp->next;
        delete tt;
    }
}

bool s6_isAlised(struct LinkNode *lnode)
{
    // 判断这个结点是否属于别名区
    struct btTreeNode *btnode = lnode->node;
    int sum = btnode->space.add.num - btnode->space.sub.num;
    int dgreef = btnode->space.add.elements[0].dgreef;
    double ratio = btnode->ratio;
    if (ratio >= 0.95 && dgreef >= 2 && sum >= 256)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int s6_speExpand(struct LinkNode *link)
{
    // 扫描空间扩展
    // *不考虑link指向的结点是起始结点，因为达不到这个量级（16^32）
    // *加集族中的各个元素的自由度是相等的，减集族中各个元素的自由度可能不等
    struct btTreeNode *btnode = link->node;
    // 1. 保存本结点的加集族信息a，即本结点下的已扫描地址集合，保存到old_add中
    struct scanFamily old_add;
    int old_card = btnode->space.add.card;
    int old_num = btnode->space.add.num;
    struct scanSet *old_elements = new struct scanSet [old_card];
    for (int i = 0; i < old_card; i++)
    {
        old_elements[i].expression = btnode->space.add.elements[i].expression;
        old_elements[i].dgreef = btnode->space.add.elements[i].dgreef;
    }
    old_add.card = old_card;
    old_add.num = old_num;
    old_add.elements = old_elements;
    
    // 判断是否要扩展到父结点，当已扫描地址集合的自由度等于扫描空间维度减1时，说明
    // 要扩展到父结点，已扫描集合的自由度是始终等同的，所以选取第一个元素即可，这里
    // 有一个规律，那就是加集族中每个元素的自由度应该是始终相等的
    if (btnode->space.add.elements[0].dgreef == btnode->space.dimension - 1)
    {
        // 2. 将本结点的字符优先级数组直接拷贝到父结点上，然后父结点根据字符优先级数组，
        //    设置dimension和expression，dimension等于字符优先级数组中大于等于
        //    level的数量，expression中要将大于等于level的位置置为'-'
        struct btTreeNode *parent = btnode->parent;
        int *cLev = btnode->space.charLev;
        int parent_dimension = 0;
        string parent_expression = btnode->space.expression;
        int parent_level = parent->level;
        for (int i = 0; i < 32; i++)
        {
            parent->space.charLev[i] = cLev[i];
            if (cLev[i] >= parent_level)
            {
                parent_dimension++;
                parent_expression[i] = '-';
            }
        }
        parent->space.dimension = parent_dimension;
        parent->space.expression = parent_expression;
        
        // 3. 取出父结点以下递归遍历到的所有结点中（第一个是READY或DONE状态）的已扫描地址集
        //    合信息，然后将这些已扫描地址集合（保存在加集族中）的信息放入父结点的减集族中
        //*这些取出来的已扫描地址集合之间要么是从属关系，要么无交集
        
        long fnum = 0;
        struct eLink *scannedSpe = s6_recScannedSpe1(parent, fnum); // parent->status == UOPEN
        int card = 0;
        struct eLink *tp = scannedSpe;
        while (tp != NULL)
        {
            card++;
            tp = tp->next;
        }
        if (parent->space.sub.card != 0) // 按理来说这个情况应该不会出现
        {
            delete [] parent->space.sub.elements;
        }
        parent->space.sub.card = card;
        parent->space.sub.elements = new struct scanSet [card];
        int num = 0;
        tp = scannedSpe;
        for (int i = 0; i < card; i++, tp = tp->next)
        {
            string expression = tp->expression;
            string sexpression = "";
            int index = s6_getSexpre(sexpression, parent_expression, expression);
            parent->space.sub.elements[i].expression = sexpression;
            parent->space.sub.elements[i].dgreef = index;
            if (index * 4 > 30)
            {
                num = (1 << 30);
            }
            else
            {
                num += (1 << (index * 4));
            }
        }
        parent->space.sub.num = num;
        
        // 4. 对本结点的加集族实施空间扩展，然后放入到父结点的加集族信息中
        parent->space.add.card = 1;
        parent->space.add.elements = new struct scanSet [1];
        int dgreef = 0;
        string parent_sexpression = "";
        string child_expression = btnode->space.expression;
        int *child_cLev = btnode->space.charLev;
        for (int i = 0; i < 32; i++)
        {
            if (child_cLev[i] == parent_level)
            {
                parent_sexpression += child_expression[i];
            }
            else if (child_cLev[i] > parent_level)
            {
                parent_sexpression += '*';
                dgreef++;
            }
        }
        parent->space.add.elements[0].dgreef = dgreef;
        parent->space.add.elements[0].expression = parent_sexpression;
        if (dgreef * 4 > 30)
        {
            parent->space.add.num = (1 << 30);
        }
        else
        {
            parent->space.add.num = (1 << (dgreef * 4));
        }
        
        // 5. 将link指向的结点更改为父结点，将父结点置为READY状态，并设置父结点的ratio为
        //    其减集族空间的AAP
        link->node = parent;
        parent->status = ___AL_STA_READY;
        if (fnum == 0)
        {
            fnum = 1;
        }
        parent->fnum = fnum;
        parent->ratio = ((double )fnum / (double )(parent->space.add.num));
        
        // 6. 将父结点下面的所有结点置为FINISH状态，并保持父结点为READY状态
        s6_recFinish(parent);
        parent->status = ___AL_STA_READY;
        
        // 7. 将scannedSpe释放
        s6_releaseELink(scannedSpe);
    }
    else
    {
        // 2. 对本结点的加集族a实施空间扩展为A(new_add)，然后根据A从兄弟结点以及自己
        //    以下的所有结点中进行递归遍历，一旦发现遍历到的结点的已扫描空间能够被A覆盖，
        //    那么就将该结点置为FINISH，并将被覆盖的已扫描空间取出来作为减集族
        //*A的信息使用eLink链表表示
        
        struct scanFamily new_add;
        // new_add要先复制成和old_add一样
        int new_card = old_add.card;
        int new_num = old_add.num;
        struct scanSet *new_elements = new struct scanSet [new_card];
        for (int i = 0; i < new_card; i++)
        {
            new_elements[i].expression = old_elements[i].expression;
            new_elements[i].dgreef = old_elements[i].dgreef;
        }
        new_add.card = new_card;
        new_add.num = new_num;
        new_add.elements = new_elements;
        // 然后将它扩展
        // 扩展的方法这里要修改，如果是叶子节点，那么要先选取字符优先级为level+1的字符进行扩展，
        // 然后再对字符优先级为level的字符进行扩展；如果不是叶子节点，那么就采用下述代码即可
        if (btnode->childrenNum == 0) // 是叶子节点
        {
            // 首先求出new_elements中的每个expression上的字符对应的字符优先级，然后根据字符优先级，
            // 首先对level+1的进行扩展，再对level的进行扩展
            string expr = btnode->space.expression;
            int *cLev = btnode->space.charLev;
            int new_cLev[32];
            int new_cLev_scale = 0;
            for (int i = 0; i < 32; i++)
            {
                if (expr[i] == '-')
                {
                    new_cLev[new_cLev_scale++] = cLev[i];
                }
            }
            // new_cLev就保存了new_elements中每个expression上字符对应的字符优先级，首先找有没有
            // 优先级是level+1并且没有被扩展的进行扩展，没有才对优先级是level的进行扩展
            int lev = btnode->level;
            int sit = -1;
            for (int i = new_cLev_scale - 1; i >= 0; i--)
            {
                if ((new_cLev[i] == lev + 1) && (new_elements[0].expression[i] != '*'))
                {
                    sit = i;
                    break;
                }
            }
            if (sit != -1)
            {
                for (int i = 0; i < new_card; i++)
                {
                    new_elements[i].expression[sit] = '*';
                }
            }
            else // 没有找到level+1的
            {
                int si = -1;
                for (int i = new_cLev_scale - 1; i >= 0; i--)
                {
                    if (new_elements[0].expression[i] != '*')
                    {
                        si = i;
                        break;
                    }
                }
                for (int i = 0; i < new_card; i++)
                {
                    new_elements[i].expression[si] = '*';
                }
            }
        }
        else // 不是叶子节点
        {
            for (int i = 0; i < new_card; i++)
            {
                int size = (int )(new_elements[i].expression.size());
                for (int j = size - 1; j >= 0; j--)
                {
                    if (new_elements[i].expression[j] != '*')
                    {
                        new_elements[i].expression[j] = '*';
                        break;
                    }
                }
            }
        }
        sort(new_elements, new_elements + new_card, s6_cmp);
        new_card = s6_unique(new_elements, new_card);
        int t_num = 0;
        for (int i = 0; i < new_card; i++)
        {
            new_elements[i].dgreef++;
            t_num += (1 << (new_elements[i].dgreef * 4));
        }
        new_add.card = new_card;
        new_add.num = t_num;
        // 至此，new_add中保存了A的信息，接下来已扩展了的空间保存成eLink链表readySpe
        // 这里readySpe链表中应该保存的是dimension维度的扫描空间表达式
        struct eLink *readySpe = NULL;
        string texpression = btnode->space.expression;
        for (int i = 0; i < new_card; i++)
        {
            string expression = new_elements[i].expression;
            if (readySpe == NULL)
            {
                readySpe = new struct eLink;
                readySpe->expression = expression;
                readySpe->next = NULL;
            }
            else
            {
                struct eLink *tel = new struct eLink;
                tel->expression = expression;
                tel->next = readySpe;
                readySpe = tel;
            }
        }
        // 接下来进行递归遍历，求出所有被覆盖了的已扫描空间，同时将相应结点置为FINISH状态
        int dimension = btnode->space.dimension;
        int dgreef = new_elements[0].dgreef;
        int level = btnode->level;
        long fnum = 0.0;
        struct eLink *scannedSpe;
        if (btnode->parent == NULL)
        {
            struct btTreeNode *new_node = new struct btTreeNode;
            new_node->children = new struct btTreeNode *[3];
            new_node->children[0] = btnode;
            new_node->childrenNum = 1;
            btnode->parent = new_node;
            scannedSpe = s6_recScannedSpe2(readySpe, btnode->parent, dimension, dgreef, level, fnum);
            btnode->parent = NULL;
            delete [] new_node->children;
            delete new_node;
        }
        else
        {
            scannedSpe = s6_recScannedSpe2(readySpe, btnode->parent, dimension, dgreef, level, fnum);
        }
        // 这里取出的scannedSpe即是应该放入减集族的完整32长度表达式，将它置入结点的减集族中
        // 之所以是取完整32长度表达式，是因为在遍历到一些比本层低2层以上的结点时，只保留部分长度表达式可能造成信息缺失
        int *cLev = btnode->space.charLev;
        int scard = 0;
        for (struct eLink *tp = scannedSpe; tp != NULL; tp = tp->next)
        {
            scard++;
        }
        if (btnode->space.sub.card != 0)
        {
            delete [] btnode->space.sub.elements;
        }
        btnode->space.sub.card = scard;
        btnode->space.sub.elements = new struct scanSet [scard];
        struct scanSet *selements = btnode->space.sub.elements;
        int snum = 0;
        int ele_tip = 0;
        for (struct eLink *tp = scannedSpe; tp != NULL; tp = tp->next)
        {
            string sexpression = tp->expression;
            string se = "";
            int dg = 0;
            for (int i = 0; i < 32; i++)
            {
                if (cLev[i] >= level)
                {
                    se += sexpression[i];
                    dg += sexpression[i] == '*'? 1 : 0;
                }
            }
            snum += (16 << ((dg - 1)*4));
            selements[ele_tip].expression = se;
            selements[ele_tip].dgreef = dg;
            ele_tip++;
        }
        btnode->space.sub.num = snum;
        
        // 3. 把A置入结点的加集族中
        if (btnode->space.add.card != 0)
        {
            delete [] btnode->space.add.elements;
        }
        btnode->space.add.card = new_add.card;
        btnode->space.add.num = new_add.num;
        btnode->space.add.elements = new_add.elements;
        
        // 4. 将结点的状态置为READY，同时保存结点的ratio为其减集族空间的AAP
        btnode->status = ___AL_STA_READY;
        if (fnum == 0)
        {
            fnum = 1;
        }
        btnode->fnum = fnum;
        btnode->ratio = ((double )fnum / (double )btnode->space.sub.num);
        
        // 5. 释放生成的动态数据
        s6_releaseELink(readySpe);
    }
    
    delete [] old_elements;
    return (link->node->space.add.num) - (link->node->space.sub.num);
}

void s6_recSubLeaf(string expression, struct snTreeNode *anode, int start_sit, int diff_num)
{
    // 从叶子结点中减去减集空间
    int diff_sit;
    string ip_addr = anode->ip_addr;
    for (diff_sit = start_sit; diff_sit < 32; diff_sit++)
    {
        if (expression[diff_sit] != ip_addr[diff_sit])
        {
            break;
        }
    }
    anode->leaf = false;
    for (int i = 0; i <= 9; i++)
    {
        char ch = '0' + i;
        ip_addr[diff_sit] = ch;
        if (diff_num == 1 && expression == ip_addr)
        {
            anode->children[i] = NULL;
        }
        else
        {
            anode->children[i] = new struct snTreeNode;
            struct snTreeNode *cnode = anode->children[i];
            ip_addr[diff_sit] = ch;
            cnode->ip_addr = ip_addr;
            cnode->dgreef = anode->dgreef - 1;
            cnode->btnode_number = anode->btnode_number;
            cnode->leaf = true;
            cnode->parent = anode;
        }
    }
    for (int i = 10; i <= 15; i++)
    {
        char ch = 'a' + i - 10;
        ip_addr[diff_sit] = ch;
        if (diff_num == 1 && expression == ip_addr)
        {
            anode->children[i] = NULL;
        }
        else
        {
            anode->children[i] = new struct snTreeNode;
            struct snTreeNode *cnode = anode->children[i];
            ip_addr[diff_sit] = ch;
            cnode->ip_addr = ip_addr;
            cnode->dgreef = anode->dgreef - 1;
            cnode->btnode_number = anode->btnode_number;
            cnode->leaf = true;
            cnode->parent = anode;
        }
    }
    diff_num--;
    if (diff_num == 0)
    {
        return ;
    }
    for (int i = 0; i < 16; i++)
    {
        if (anode->children[i] != NULL && s5_belong(expression, anode->children[i]))
        {
            s6_recSubLeaf(expression, anode->children[i], diff_sit + 1, diff_num);
        }
    }
}

void s6_recSubForest(string expression, struct snTreeNode *anode)
{
    // expression属于anode的子网
    if (anode->leaf == true)
    {
        // anode->ip_addr: 56*****78
        // expression:     56*2*4*78
        string ip_addr = anode->ip_addr;
        int diff_num = 0; // 2
        for (int i = 0; i < 32; i++)
        {
            if (expression[i] != ip_addr[i])
            {
                diff_num++;
            }
        }
        if (diff_num == 0)
        {
            struct snTreeNode *pnode = anode->parent;
            for (int i = 0; i < 16; i++)
            {
                if (pnode->children[i] == anode)
                {
                    pnode->children[i] = NULL;
                    break;
                }
            }
            s5_recReleaseSnTree(anode);
        }
        else
        {
            s6_recSubLeaf(expression, anode, 0, diff_num);
        }
        return ;
    }
    for (int i = 0; i < 16; i++)
    {
        if (anode->children[i] != NULL && s5_belong(expression, anode->children[i]))
        {
            s6_recSubForest(expression, anode->children[i]);
            break;
        }
    }
}

int s6_linkScan(struct LinkNode *first, struct LinkNode *last)
{
    // 里面的所有结点都已经扩展了空间
    // objects_scale等于这次扫描将扫描的地址数量，扫描完成后要加到tsnum中，参
    // 照leavesScan函数在最后扫描完毕之后注意要更新tfnum和tsnum，同时更新各个结点的状态
    // 同时别名区探测也是在这里做的
    // 扫描完毕之后还要输出扫描结果
    long objects_scale = 0;
    struct LinkNode *pnode = first;
    while (true)
    {
        struct btTreeNode *btnode = pnode->node;
        if (btnode->status != ___AL_STA_READY)
        {
            if (pnode == last)
            {
                break;
            }
            else
            {
                pnode = pnode->next;
                continue;
            }
        }
        // 计算每个结点将扫描的地址数量，然后累加到objects_scale中
        objects_scale += (btnode->space.add.num) - (btnode->space.sub.num);
        if (pnode == last)
        {
            break;
        }
        else
        {
            pnode = pnode->next;
        }
    }
#if ___ENV == ___ENV_TEST
    // 模拟扫描
    long fnum = 0; // 本次扫描中累计发现的地址个数
    struct LinkNode *i = first;
    int len = 1; // 从first到last的结点个数
    while (true)
    {
        struct btTreeNode *btnode = i->node;
        if (btnode->status != ___AL_STA_READY)
        {
            if (i == last)
            {
                break;
            }
            else
            {
                i = i->next;
                len++;
                continue;
            }
        }
        int acard = btnode->space.add.card;
        int scard = btnode->space.sub.card;
        struct scanSet *aelements = btnode->space.add.elements;
        struct scanSet *selements = btnode->space.sub.elements;
        string expression = btnode->space.expression;
        int lfnum = 0; // 特定结点下发现的地址个数
        for (int j = 0; j < acard; j++)
        {
            string sspace = aelements[j].expression;
            int tip = 0;
            string cspace = "";
            for (int k = 0; k < 32; k++)
            {
                if (expression[k] == '-')
                {
                    cspace += sspace[tip++];
                }
                else
                {
                    cspace += expression[k];
                }
            }
            // results << cspace << endl;
            int rnum = s5_recSearch(cspace, &seXree, 32);
            lfnum += rnum;
            fnum += (long )rnum;
        }
        for (int j = 0; j < scard; j++)
        {
            string sspace = selements[j].expression;
            int tip = 0;
            string cspace = "";
            for (int k = 0; k < 32; k++)
            {
                if (expression[k] == '-')
                {
                    cspace += sspace[tip++];
                }
                else
                {
                    cspace += expression[k];
                }
            }
            int rnum = s5_recSearch(cspace, &seXree, 32);
            lfnum -= rnum;
            fnum -= rnum;
        }
        btnode->status = ___AL_STA_DONE;
        int lsnum = btnode->space.add.num - btnode->space.sub.num;
        if (lsnum == 0)
        {
            // btnode->ratio = ((btnode->ratio) * (btnode->space.sub.num)) / (btnode->space.add.num);
            // ratio不变
            btnode->fnum = (btnode->ratio) * (btnode->space.sub.num);
        }
        else
        {
            btnode->fnum = (btnode->ratio) * (btnode->space.sub.num) + ((double )(lfnum));
            btnode->ratio = (btnode->fnum) / (btnode->space.add.num);
        }
        if (i == last)
        {
            break;
        }
        else
        {
            i = i->next;
            len++;
        }
    }
    tfnum += fnum;
    tsnum += objects_scale;
    results << "      --Dynamic Scanning--" << endl;
    results << "Active Number: " << fnum << endl;
    results << "Scanning Number: " << objects_scale << endl;
    results << "Total Active Number: " << tfnum << endl;
    results << "Total Scanning Number: " << tsnum << endl;
    cout << "      --Dynamic Scanning--" << endl;
    cout << "Active Number: " << fnum << endl;
    cout << "Scanning Number: " << objects_scale << endl;
    cout << "Total Active Number: " << tfnum << endl;
    cout << "Total Scanning Number: " << tsnum << endl;
    return len;
#elif ___ENV == ___ENV_WORK
    // 进行实际扫描，遍历[left, right]范围内的结点，对每个结点，将它的待扫描空间提取出来，
    // 保存到一个临时文档中，然后调用ZMapv6扫描，并将活跃地址数量统计出来
    
    // 1. 依次遍历[left, right]的每个结点，每遍历到一个结点，根据其加集族和减集族信息生成
    //    子网森林，然后将其中所有地址保存到targetArr中，这样遍历完所有结点
    int len = 1;
    targetArr_scale = objects_scale;
    targetArr_tip = 0;
    targetArr = new string [targetArr_scale + 16];
    snXree_scale = 0;
    struct LinkNode *inode = first;
    while (true)
    {
        struct btTreeNode *btnode = inode->node;
        if (btnode->status != ___AL_STA_READY)
        {
            if (inode == last)
            {
                break;
            }
            else
            {
                inode = inode->next;
                len++;
                continue;
            }
        }
        snXree_scale += btnode->space.add.card;
        if (inode == last)
        {
            break;
        }
        else
        {
            inode = inode->next;
            len++;
        }
    }
    snXree = new struct snTreeNode *[snXree_scale + 16];
    snXree_tip = 0;
    inode = first;
    while (true)
    {
        struct btTreeNode *btnode = inode->node;
        if (btnode->status != ___AL_STA_READY)
        {
            if (inode == last)
            {
                break;
            }
            else
            {
                inode = inode->next;
                continue;
            }
        }
        btnode->fnum = 0;
        string texpression = btnode->space.expression;
        // 1.1 将该结点的加集族信息保存到子网森林中
        int acard = btnode->space.add.card;
        int snXree_left = snXree_tip; // 这个结点包含的子网森林左界
        for (int i = 0; i < acard; i++)
        {
            string sexpression = btnode->space.add.elements[i].expression;
            string expression = "";
            // 根据texpression和sexpression生成初步的子网表达式
            s5_fuseExpr(expression, sexpression, texpression);
            // expression即是初步的子网表达式，它的形式可能是56*78**这样的字符串，
            // 字符串中部也可能会有自由度符号，要根据它生成自由符仅在尾部的子网表达式，
            // 并用子网树的形式表示出来
            snXree[snXree_tip] = new struct snTreeNode;
            struct snTreeNode *snnode = snXree[snXree_tip];
            snXree_tip++;
            snnode->ip_addr = expression;
            snnode->dgreef = btnode->space.add.elements[i].dgreef;
            snnode->btnode_number = btnode->number;
            snnode->leaf = true;
            snnode->parent = NULL;
        }
        int snXree_right = snXree_tip - 1; // 这个结点包含的子网森林右界
        // 1.2 将该结点的减集族信息更新到这个结点的子网森林中
        int scard = btnode->space.sub.card;
        for (int i = 0; i < scard; i++)
        {
            string sexpression = btnode->space.sub.elements[i].expression;
            string expression = "";
            s5_fuseExpr(expression, sexpression, texpression);
            for (int j = snXree_left; j <= snXree_right; j++)
            {
                if (s5_belong(expression, snXree[j]) == true)
                {
                    s6_recSubForest(expression, snXree[j]);
                    break;
                }
            }
        }
        if (inode == last)
        {
            break;
        }
        else
        {
            inode = inode->next;
        }
    }
    
    // 2. 从子网森林写入地址到targetArr，然后将其中的所有地址随机化后写入到target文件中
    for (int i = 0; i < snXree_scale; i++)
    {
        struct snTreeNode *snnode = snXree[i];
        s5_recInputAddr(snnode);
    }
    s5_RandomGenerator sg;
    random_shuffle(targetArr, targetArr + objects_scale, sg);
    // 将targetArr数组中的元素转换为标准IPv6表达格式，然后保存到target文件中
    ofstream targetfile;
    targetfile.open(___FL_TARGET);
    for (long i = 0; i < objects_scale; i++)
    {
        targetfile << s5_tranIPv6(targetArr[i]) << endl;
    }
    targetfile.close();
    
    /*// 3. 调用ZMapv6进行扫描
    string ins = string(___PR_ZMAP) + " --probe-module=icmp6_echoscan --ipv6-target-file=" + string(___FL_TARGET);
    ins += " --ipv6-source-ip=" + string(___PR_SOURCE) + " --bandwidth=" + string(___PR_BAND);
    ins += " --output-file=" + string(___FL_ZRES) + " --cooldown-time=" + string(___PR_CTIME);
    system(ins.c_str());
    */
    // 3. Use Icmpv6
    string ins = "python2 ping6.py " + string(___FL_TARGET) +" "+ string(___FL_ZRES);
    system(ins.c_str());

    // 4. 统计扫描情况，更新各个结点的信息，包括其ratio值和status值
    string *active; // 存放活跃IPv6地址的数组，下标从0开始，以完全16进制格式表示
    int active_scale = 0; // 活跃IPv6地址的数量
    ifstream resfile;
    resfile.open(___FL_ZRES);
    string line;
    while (getline(resfile, line))
    {
        active_scale++;
    }
    resfile.close();
    active = new string [active_scale + 16];
    active_scale = 0;
    resfile.open(___FL_ZRES);
    while (getline(resfile, line))
    {
        string ipv6Nm = "";
        s5_ipv6Normalize(line, ipv6Nm);
        active[active_scale++] = ipv6Nm;
    }
    resfile.close();
    sort(active, active + active_scale, s4_cmp);
    sort(snXree, snXree + snXree_scale, s5_snCmp);
    snXree_tip = 0;;
    for (long i = 0; i < active_scale; i++)
    {
        string aip = active[i];
        for (; snXree_tip < snXree_scale; snXree_tip++)
        {
            if (s5_belong(aip, snXree[snXree_tip]) == true)
            {
                int btnode_number = snXree[snXree_tip]->btnode_number;
                btnodes[btnode_number]->fnum += 1;
                break;
            }
        }
    }
    inode = first;
    while (true)
    {
        struct btTreeNode *btnode = inode->node;
        if (btnode->status != ___AL_STA_READY)
        {
            if (inode == last)
            {
                break;
            }
            else
            {
                inode = inode->next;
                continue;
            }
        }
        int num = btnode->space.add.num - btnode->space.sub.num;
        long fn = btnode->fnum;
        if (num == 0)
        {
            // btnode->ratio = ((btnode->ratio) * (btnode->space.sub.num)) / (btnode->space.add.num);
            // ratio不变
        }
        else
        {
            btnode->ratio = ((btnode->ratio) * (btnode->space.sub.num) + ((double )(fn))) / (btnode->space.add.num);
        }
        btnode->status = ___AL_STA_DONE;
        if (inode == last)
        {
            break;
        }
        else
        {
            inode = inode->next;
        }
    }
    
    // 5. 输出统计结果
    tfnum += active_scale;
    tsnum += objects_scale;
    results << "      --Dynamic Scanning--" << endl;
    results << "Active Number: " << active_scale << endl;
    results << "Scanning Number: " << objects_scale << endl;
    results << "Total Active Number: " << tfnum << endl;
    results << "Total Scanning Number: " << tsnum << endl;
    for (int i = 0; i < active_scale; i++)
    {
        list << active[i] << endl;
    }
    cout << "      --Dynamic Scanning--" << endl;
    cout << "Active Number: " << active_scale << endl;
    cout << "Scanning Number: " << objects_scale << endl;
    cout << "Total Active Number: " << tfnum << endl;
    cout << "Total Scanning Number: " << tsnum << endl;
    
    // 6. 释放动态数据
    delete [] active;
    for (int i = 0; i < snXree_scale; i++)
    {
        s5_recReleaseSnTree(snXree[i]);
    }
    delete [] snXree;
    delete [] targetArr;
    return len;
#endif
}

void s6_insert(struct LinkNode *end, struct LinkNode *lknode)
{
    // 根据ratio值，将node插入到end后面的链表中去
    if (end->next == NULL)
    {
        end->next = lknode;
        lknode->next = NULL;
        return ;
    }
    struct LinkNode *sit = end->next;
    struct LinkNode *last = end;
    while (sit->node->ratio > lknode->node->ratio)
    {
        if (sit->next == NULL)
        {
            sit->next = lknode;
            lknode->next = NULL;
            return ;
        }
        else
        {
            last = sit;
            sit = sit->next;
        }
    }
    last->next = lknode;
    lknode->next = sit;
}

#if ___ENV == ___ENV_WORK
void s6_addCopy(struct scanFamily &alise, struct btTreeNode *btnode)
{
    // 保存当前加集族信息
    // 这里不统计num和dgreef情况，另外需要求出完整的地址表达式，因此alise的数据内容和
    // 一般的scanFamily数据结构是有区别的
    if (alise.elements != NULL)
    {
        delete [] alise.elements;
    }
    int card = btnode->space.add.card;
    int num = 0;
    alise.card = card;
    alise.elements = new struct scanSet [card];
    string texpression = btnode->space.expression;
    for (int i = 0; i < card; i++)
    {
        string sexpression = btnode->space.add.elements[i].expression;
        int dgreef = btnode->space.add.elements[i].dgreef;
        num += (1 << (dgreef * 4));
        string expression = "";
        s5_fuseExpr(expression, sexpression, texpression);
        alise.elements[i].expression = expression;
        alise.elements[i].dgreef = dgreef;
    }
    alise.num = num;
}
#endif

void s6_addOutput(struct scanFamily &alise, struct btTreeNode *btnode)
{
    // 输出加集族信息
    alias << "Alias Information - Node " << btnode->number << ":" << endl;
    cout << "Alias Information - Node " << btnode->number << endl;
    int card = alise.card;
    for (int i = 0; i < card; i++)
    {
        alias << alise.elements[i].expression << endl;
    }
}

#if ___ENV == ___ENV_WORK
string s6_randomAddr(struct snTreeNode **snXree)
{
    // 16进制情况
    // 从子网森林中随机选取出一个地址，返回时是完全16进制形式的
    struct snTreeNode *snnode = NULL;
    while (true)
    {
        int sit = rand() % snXree_scale;
        snnode = snXree[sit];
        while (snnode->leaf == false)
        {
            bool testnull = true;
            for (int i = 0; i < 16; i++)
            {
                if (snnode->children[i] != NULL)
                {
                    testnull = false;
                    break;
                }
            }
            if (testnull == true)
            {
                break;
            }
            
            int csit = 0;
            do
            {
                csit = rand() % 16;
            } while (snnode->children[csit] == NULL);
            snnode = snnode->children[csit];
        }
        if (snnode->leaf == true)
        {
            break;
        }
    }
    string ip_addr = snnode->ip_addr;
    for (int i = 0; i < 32; i++)
    {
        if (ip_addr[i] == '*')
        {
            int r = rand() % 16;
            if (r >= 0 && r <= 9)
            {
                ip_addr[i] = '0' + r;
            }
            else // r >= 10 && r <= 15
            {
                ip_addr[i] = 'a' + r - 10;
            }
        }
    }
    return ip_addr;
}
#endif

#if ___ENV == ___ENV_WORK
void s6_aliExpand(struct LinkNode *anode)
{
    // 别名区扩展
    // 目前的anode结点指向的回溯树结点是已经没有进行空间扩展的
    struct scanFamily alise;
    alise.elements = NULL;
    alise.card = 0;
    // 保存当前加集族信息
    s6_addCopy(alise, anode->node);
    // 进行扩展
    s6_speExpand(anode);
    while (true)
    {
        // 1. 调用Zmap随机扫描设定数量的地址
        // 1.1 生成待扫描子网森林
        struct btTreeNode *btnode = anode->node;
        int acard = btnode->space.add.card;
        snXree_scale = acard;
        snXree = new struct snTreeNode *[snXree_scale + 16];
        snXree_tip = 0;
        string texpression = btnode->space.expression;
        for (int i = 0; i < acard; i++)
        {
            string sexpression = btnode->space.add.elements[i].expression;
            string expression = "";
            s5_fuseExpr(expression, sexpression, texpression);
            snXree[snXree_tip] = new struct snTreeNode;
            struct snTreeNode *snnode = snXree[snXree_tip];
            snXree_tip++;
            snnode->ip_addr = expression;
            snnode->dgreef = btnode->space.add.elements[i].dgreef;
            snnode->btnode_number = btnode->number;
            snnode->leaf = true;
            snnode->parent = NULL;
        }
        int snXree_left = 0;
        int snXree_right = snXree_scale - 1;
        int scard = btnode->space.sub.card;
        for (int i = 0; i < scard; i++)
        {
            string sexpression = btnode->space.sub.elements[i].expression;
            string expression = "";
            s5_fuseExpr(expression, sexpression, texpression);
            for (int j = snXree_left; j <= snXree_right; j++)
            {
                if (s5_belong(expression, snXree[j]) == true)
                {
                    s6_recSubForest(expression, snXree[j]);
                    break;
                }
            }
        }
        // 1.2 从待扫描子网森林中选取一定数量的地址写入到文件中并执行扫描
        ofstream targetfile;
        targetfile.open(___FL_TARGET);
        for (long i = 0; i < ___PR_ALITEST; i++)
        {
            targetfile << s5_tranIPv6(s6_randomAddr(snXree)) << endl;
        }
        targetfile.close();
        /*string ins = string(___PR_ZMAP) + " --probe-module=icmp6_echoscan --ipv6-target-file=" + string(___FL_TARGET);
        ins += " --ipv6-source-ip=" + string(___PR_SOURCE) + " --bandwidth=" + string(___PR_ALI_BAND);
        ins += " --output-file=" + string(___FL_ZRES)+ " --cooldown-time=" + string(___PR_ALI_CTIME);
        system(ins.c_str());
        */
        // Use Icmpv6
        string ins = "python2 ping6.py " + string(___FL_TARGET) +" "+ string(___FL_ZRES);
        system(ins.c_str());
        // 2. 释放动态生成的数据
        for (int i = 0; i < snXree_scale; i++)
        {
            s5_recReleaseSnTree(snXree[i]);
        }
        delete [] snXree;
        
        // 3. 统计活跃地址数量，如果等于设定的数量，那么再扩展一遍，否则退出
        int active_scale = 0;
        ifstream resfile;
        resfile.open(___FL_ZRES);
        string line;
        while (getline(resfile, line))
        {
            active_scale++;
        }
        resfile.close();
        if (active_scale >= ___PR_ALITEST * 0.95)
        {
            // 保存当前加集族信息
            s6_addCopy(alise, anode->node);
            // 进行扩展
            s6_speExpand(anode);
        }
        else
        {
            // 输出加集族信息
            s6_addOutput(alise, anode->node);
            break;
        }
    }
}
#endif

int s6_lnCmp(struct LinkNode *ln1, struct LinkNode *ln2)
{
    return (ln1->node->ratio - ln2->node->ratio) > 0;
}

void s6_recwriteLeafAddr(struct seTreeNode *sit)
{
    // 遍历当前查询树，将其中的所有叶子结点地址写入到snow文件中
    if (sit->leaf == true)
    {
        snow_list[snow_list_tip++] = sit->leaf_addr;
    }
    else
    {
        for (int i = 0; i < 16; i++)
        {
            if (sit->children[i] != NULL)
            {
                s6_recwriteLeafAddr(sit->children[i]);
            }
        }
    }
}

void s6_recwriteLeafSnow(string spe, struct seTreeNode *sit, int spe_size)
{
    if (spe[0] == '*')
    {
        bool pure = true;
        for (int i = 1; i < spe_size; i++)
        {
            if (spe[i] != '*')
            {
                pure = false;
            }
        }
        if (pure == true)
        {
            s6_recwriteLeafAddr(sit);
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                if (sit->children[i] != NULL)
                {
                    s6_recwriteLeafSnow(spe.substr(1), sit->children[i], spe_size - 1);
                }
            }
        }
    }
    else
    {
        int lo = spe[0] >= 'a' ? spe[0] - 'a' + 10 : spe[0] - '0';
        if (spe_size == 1 && sit->children[lo] != NULL)
        {
            snow_list[snow_list_tip++] = sit->children[lo]->leaf_addr;
            return ;
        }
        if (sit->children[lo] == NULL)
        {
            return ;
        }
        else
        {
            s6_recwriteLeafSnow(spe.substr(1), sit->children[lo], spe_size - 1);
        }
    }
}

void s6_recwriteSnow(struct snTreeNode *swnode)
{
    if (swnode->leaf == true)
    {
        s6_recwriteLeafSnow(swnode->ip_addr, &seXree, 32);
        return ;
    }
    for (int i = 0; i < 16; i++)
    {
        if (swnode->children[i] != NULL)
        {
            s6_recwriteSnow(swnode->children[i]);
        }
    }
}

void s6_snowSta(string snowf)
{
    // 从DONE状态的结点中提取出所有的子网森林空间，然后基于这些空间从查询树中将地址取出来
    
    // 1. 遍历链表中的每个结点，并对DONE状态结点进行操作
    
    // 2. 对每个DONE状态结点，首先取出其加空间集合，然后用减空间集合去进行减操作，进过减操作后
    //    形成的剩余空间是一个子网森林，遍历这个子网森林并对其中的每棵树，结合查询树结构将所有
    //    其中的地址提取出来保存到snow文件中
    
    snow_list_scale = tfnum;
    snow_list_tip = 0;
    snow_list = new string [snow_list_scale];
    struct LinkNode *ti = liXree;
    while (ti != NULL)
    {
        if (ti->node->status == ___AL_STA_DONE)
        {
            struct btTreeNode *btnode = ti->node;
            struct snTreeNode **swXree;
            int swXree_scale = btnode->space.add.card;
            swXree = new struct snTreeNode *[swXree_scale + 2];
            int swXree_tip = 0;
            string texpression = btnode->space.expression;
            for (int i = 0; i < swXree_scale; i++)
            {
                string sexpression = btnode->space.add.elements[i].expression;
                string expression = "";
                s5_fuseExpr(expression, sexpression, texpression);
                swXree[swXree_tip] = new struct snTreeNode;
                struct snTreeNode *swnode = swXree[swXree_tip];
                swXree_tip++;
                swnode->ip_addr = expression;
                swnode->dgreef = btnode->space.add.elements[i].dgreef;
                swnode->btnode_number = btnode->number;
                swnode->leaf = true;
                swnode->parent = NULL;
            }
            // 将swXree子网森林中的每个表达式依次取出来，然后根据查询树找出
            // 结果IPv6地址并写入到snow文件中
            for (int i = 0; i < swXree_scale; i++)
            {
                s6_recwriteSnow(swXree[i]);
                s5_recReleaseSnTree(swXree[i]);
            }
            delete [] swXree;
        }
        ti = ti->next;
    }
    sort(snow_list, snow_list + snow_list_scale, s4_cmp);
    snow.open(snowf);
    for (long i = 0; i < snow_list_scale; i++)
    {
        snow << snow_list[i] << endl;
    }
    snow.close();
    delete [] snow_list;
}

void s6_dynamicScan(int ttnum, string snowf)
{
    // 动态扫描
    // 从链表头部开始，依次对每个结点进行空间扩展，直到超过扫描触发阈值，然后对积累
    // 的结点进行扫描，扫描完毕之后：统计扫描地址数量并决定是否停止扫描、统计各个结
    // 点的扫描活跃地址比例ratio，然后根据ratio将它们重新放入链表中、如果ratio在
    // 结点扫描地址大于256的情况下仍然为1，那么判定为别名区，并进行别名区探测
    ___al_scan_ttnum = ttnum;
    
    while (true)
    {
        struct LinkNode *first; // 待扫描链表段中的头结点
        struct LinkNode *last; // 待扫描链表段中的尾结点
        long sum = 0; // 这个链表段累计会扫描的地址数量
        
        // 找到第一个处于DONE状态并且不是别名区结点的结点，将其初始化为待扫描链表段
        // 中的第一个结点
        struct LinkNode *ti = liXree;
        while (true)
        {
            if (ti->node->status != ___AL_STA_DONE) // status == ___AL_STA_FINISH
            {
                // 将这个结点删除
                struct LinkNode *tj = ti;
                ti = ti->next;
                delete tj;
            }
#if ___ENV == ___ENV_WORK
            else if (s6_isAlised(ti))
            {
                // 对其进行别名区扩展，然后将结点ratio重置，status置为DONE，插入到后续链表中
                // 输出别名区信息
                s6_aliExpand(ti);
                // 别名区结点在识别出来之后，其ratio值不要设置为0.0，这样其永远不会被访问到，而是应该设置成1除以空间
                // 规模（假设整个别名区代表一个主机）
                ti->node->ratio = 1.0 / (ti->node->space.add.num);
                ti->node->status = ___AL_STA_DONE;
                struct LinkNode *tj = ti->next;
                s6_insert(tj, ti);
                ti = tj;
            }
#endif
            else
            {
                liXree = ti;
                first = ti;
                last = ti;
                sum += (long )s6_speExpand(ti);
                break;
            }
        }
        // 不断增添结点，直到触发扫描作业
        while (sum < ___AL_SCAN_THD)
        {
            ti = last->next;
            if (ti == NULL)
            {
                break;
            }
            if (ti->node->status != ___AL_STA_DONE) // status == ___AL_STA_FINISH
            {
                last->next = ti->next;
                delete ti;
                ti = last->next;
            }
#if ___ENV == ___ENV_WORK
            else if (s6_isAlised(ti))
            {
                s6_aliExpand(ti);
                // 别名区结点在识别出来之后，其ratio值不要设置为0.0，这样其永远不会被访问到，而是应该设置成1除以空间
                // 规模（假设整个别名区代表一个主机）
                ti->node->ratio = 1.0 / (ti->node->space.add.num);
                ti->node->status = ___AL_STA_DONE;
                last->next = ti->next;
                s6_insert(last->next, ti);
                ti = last->next;
            }
#endif
            else
            {
                sum += (long )s6_speExpand(ti);
                last = ti;
            }
        }
        // 对[first, last]范围内的结点执行扫描，更新链表，并判断是否达到扫描上限
        int len = s6_linkScan(first, last);
        if (tsnum > ___al_scan_ttnum)
        {
            // 如果达到了扫描数量上限，那么就退出
#if ___SNOW == ___SNOW_YES
            // 当采用snowballing模式时，将发现的所有IPv6地址写入到文件中
            cout << "Snow: Record Data" << endl;
            s6_snowSta(snowf);
            cout << "Snow: Finished" << endl;
#endif
            break;
        }
        else
        {
            // 将[first, last]范围内的仍处于DONE状态的结点根据ratio插回链表中，
            // 并更新链表使得liXree指向新链表的头结点
            struct LinkNode **arr = new struct LinkNode *[len + 2];
            int arr_scale = 0;
            struct LinkNode *sit = first;
            struct LinkNode *nex;
            for (int i = 0; i < len - 1; i++)
            {
                nex = sit->next;
                if (sit->node->status == ___AL_STA_DONE)
                {
                    arr[arr_scale++] = sit;
                }
                else
                {
                    delete sit;
                }
                sit = nex;
            }
            liXree = last->next;
            if (sit->node->status == ___AL_STA_DONE) // sit == last
            {
                arr[arr_scale++] = sit;
            }
            else
            {
                delete sit;
            }
            // 对arr进行排序，然后再进行归并排序
            // 此时的liXree有可能是NULL，如果是NULL，那么就直接将arr生成链表，否则进行归并排序
            sort(arr, arr + arr_scale, s6_lnCmp);
            if (liXree == NULL)
            {
                // 将arr生成链表
                liXree = arr[0];
                for (int i = 0; i < arr_scale - 1; i++)
                {
                    arr[i]->next = arr[i + 1];
                }
                arr[arr_scale - 1]->next = NULL;
            }
            else
            {
                // 用归并排序的方式将arr中的元素依次放入liXree中
                // 要归并的有两个，liXree链表和arr数组
                struct LinkNode *begin;
                struct LinkNode *lsit = liXree;
                int rsit = 0;
                if (lsit->node->ratio < arr[rsit]->node->ratio)
                {
                    begin = arr[rsit++];
                }
                else
                {
                    begin = lsit;
                    lsit = lsit->next;
                }
                sit = begin;
                while (lsit != NULL && rsit != arr_scale)
                {
                    if (lsit->node->ratio < arr[rsit]->node->ratio)
                    {
                        sit->next = arr[rsit++];
                    }
                    else
                    {
                        sit->next = lsit;
                        lsit = lsit->next;
                    }
                    sit = sit->next;
                }
                if (lsit != NULL && rsit == arr_scale)
                {
                    sit->next = lsit;
                }
                else if (lsit == NULL && rsit != arr_scale)
                {
                    while (rsit < arr_scale)
                    {
                        sit->next = arr[rsit++];
                        sit = sit->next;
                    }
                    sit->next = NULL;
                }
                else // lsit == NULL && rsit == arr_scale
                {
                    sit->next = NULL;
                }
                liXree = begin;
            }
            delete [] arr;
        }
    }
}

void s6_dynamicScan()
{
    // 动态扫描
    // 从链表头部开始，依次对每个结点进行空间扩展，直到超过扫描触发阈值，然后对积累
    // 的结点进行扫描，扫描完毕之后：统计扫描地址数量并决定是否停止扫描、统计各个结
    // 点的扫描活跃地址比例ratio，然后根据ratio将它们重新放入链表中、如果ratio在
    // 结点扫描地址大于256的情况下仍然为1，那么判定为别名区，并进行别名区探测
    
    while (true)
    {
        struct LinkNode *first; // 待扫描链表段中的头结点
        struct LinkNode *last; // 待扫描链表段中的尾结点
        long sum = 0; // 这个链表段累计会扫描的地址数量
        
        // 找到第一个处于DONE状态并且不是别名区结点的结点，将其初始化为待扫描链表段
        // 中的第一个结点
        struct LinkNode *ti = liXree;
        while (true)
        {
            if (ti->node->status != ___AL_STA_DONE) // status == ___AL_STA_FINISH
            {
                // 将这个结点删除
                struct LinkNode *tj = ti;
                ti = ti->next;
                delete tj;
            }
#if ___ENV == ___ENV_WORK
            else if (s6_isAlised(ti))
            {
                // 对其进行别名区扩展，然后将结点ratio重置，status置为DONE，插入到后续链表中
                // 输出别名区信息
                s6_aliExpand(ti);
                // 别名区结点在识别出来之后，其ratio值不要设置为0.0，这样其永远不会被访问到，而是应该设置成1除以空间
                // 规模（假设整个别名区代表一个主机）
                ti->node->ratio = 1.0 / (ti->node->space.add.num);
                ti->node->status = ___AL_STA_DONE;
                struct LinkNode *tj = ti->next;
                s6_insert(tj, ti);
                ti = tj;
            }
#endif
            else
            {
                liXree = ti;
                first = ti;
                last = ti;
                sum += (long )s6_speExpand(ti);
                break;
            }
        }
        // 不断增添结点，直到触发扫描作业
        while (sum < ___AL_SCAN_THD)
        {
            ti = last->next;
            if (ti == NULL)
            {
                break;
            }
            if (ti->node->status != ___AL_STA_DONE) // status == ___AL_STA_FINISH
            {
                last->next = ti->next;
                delete ti;
                ti = last->next;
            }
#if ___ENV == ___ENV_WORK
            else if (s6_isAlised(ti))
            {
                s6_aliExpand(ti);
                // 别名区结点在识别出来之后，其ratio值不要设置为0.0，这样其永远不会被访问到，而是应该设置成1除以空间
                // 规模（假设整个别名区代表一个主机）
                ti->node->ratio = 1.0 / (ti->node->space.add.num);
                ti->node->status = ___AL_STA_DONE;
                last->next = ti->next;
                s6_insert(last->next, ti);
                ti = last->next;
            }
#endif
            else
            {
                sum += (long )s6_speExpand(ti);
                last = ti;
            }
        }
        // 对[first, last]范围内的结点执行扫描，更新链表，并判断是否达到扫描上限
        int len = s6_linkScan(first, last);
        if (tsnum > ___AL_SCAN_TTNUM)
        {
            // 如果达到了扫描数量上限，那么就退出
#if ___SNOW == ___SNOW_YES
            // 当采用snowballing模式时，将发现的所有IPv6地址写入到文件中
            // 方法：将链表中的所有DONE状态结点取出来，然后通过子网森林的数据结构提取出结果数据
            cout << "Snow: Record Data" << endl;
            s6_snowSta(string(___FL_SNOW));
            cout << "Snow: Finished" << endl;
#endif
            break;
        }
        else
        {
            // 将[first, last]范围内的仍处于DONE状态的结点根据ratio插回链表中，
            // 并更新链表使得liXree指向新链表的头结点
            struct LinkNode **arr = new struct LinkNode *[len + 2];
            int arr_scale = 0;
            struct LinkNode *sit = first;
            struct LinkNode *nex;
            for (int i = 0; i < len - 1; i++)
            {
                nex = sit->next;
                if (sit->node->status == ___AL_STA_DONE)
                {
                    arr[arr_scale++] = sit;
                }
                else
                {
                    delete sit;
                }
                sit = nex;
            }
            liXree = last->next;
            if (sit->node->status == ___AL_STA_DONE) // sit == last
            {
                arr[arr_scale++] = sit;
            }
            else
            {
                delete sit;
            }
            // 对arr进行排序，然后再进行归并排序
            // 此时的liXree有可能是NULL，如果是NULL，那么就直接将arr生成链表，否则进行归并排序
            sort(arr, arr + arr_scale, s6_lnCmp);
            if (liXree == NULL)
            {
                // 将arr生成链表
                liXree = arr[0];
                for (int i = 0; i < arr_scale - 1; i++)
                {
                    arr[i]->next = arr[i + 1];
                }
                arr[arr_scale - 1]->next = NULL;
            }
            else
            {
                // 用归并排序的方式将arr中的元素依次放入liXree中
                // 要归并的有两个，liXree链表和arr数组
                struct LinkNode *begin;
                struct LinkNode *lsit = liXree;
                int rsit = 0;
                if (lsit->node->ratio < arr[rsit]->node->ratio)
                {
                    begin = arr[rsit++];
                }
                else
                {
                    begin = lsit;
                    lsit = lsit->next;
                }
                sit = begin;
                while (lsit != NULL && rsit != arr_scale)
                {
                    if (lsit->node->ratio < arr[rsit]->node->ratio)
                    {
                        sit->next = arr[rsit++];
                    }
                    else
                    {
                        sit->next = lsit;
                        lsit = lsit->next;
                    }
                    sit = sit->next;
                }
                if (lsit != NULL && rsit == arr_scale)
                {
                    sit->next = lsit;
                }
                else if (lsit == NULL && rsit != arr_scale)
                {
                    while (rsit < arr_scale)
                    {
                        sit->next = arr[rsit++];
                        sit = sit->next;
                    }
                    sit->next = NULL;
                }
                else // lsit == NULL && rsit == arr_scale
                {
                    sit->next = NULL;
                }
                liXree = begin;
            }
            delete [] arr;
        }
    }
}

int s6_recstaVisualnum(struct btTreeNode *node)
{
    if (node->childrenNum == 0)
    {
        node->visual_num = 1;
        return 1;
    }
    int sum = 0;
    for (int i = 0; i < node->childrenNum; i++)
    {
        sum += s6_recstaVisualnum(node->children[i]);
    }
    // 将结点自己也加进去，测试效果
    sum++;
    node->visual_num = sum;
    return sum;
}

void s6_storeVisual()
{
    // 保存树结构信息，包括每个结点下面所有的结点个数（包括它自己）
    s6_recstaVisualnum(&btXree);
    ofstream vis;
    vis.open("./output/m4_visual");
    vis << "Source,Target,Number,Color,Radius" << endl;
    for (int i = 1; i <= btnodes_scale; i++)
    {
        struct btTreeNode *node = btnodes[i];
        int num = node->childrenNum;
        for (int j = 0; j < num; j++)
        {
            int l = 0;
            if (node->children[j]->parent == NULL)
            {
                l = 128;
            }
            else
            {
                string exp = node->children[j]->parent->space.expression;
                for (int k = 0; k < 32; k++)
                {
                    if (exp[k] == '-')
                    {
                        l += 4;
                    }
                }
            }
            double color;
            if (node->children[j]->fnum == 0)
            {
                cout << node->children[j]->childrenNum << endl;
                cout << "test" << endl;
                color = (double )l;
            }
            else
            {
                color =  ((double )(l) - log2((double )(node->children[j]->fnum)));
            }
            // 根据该结点下面的相同取值维度的个数来设置半径
            int radius = 0;
            if (node->children[j]->childrenNum == 0)
            {
                radius = 32;
            }
            else
            {
                for (int k = 0; k < 32; k++)
                {
                    if (node->children[j]->space.expression[k] != '-')
                    {
                        radius++;
                    }
                }
            }
            vis << node->number << "," << node->children[j]->number;
            vis << "," << node->children[j]->visual_num << "," << color;
            vis << "," << radius << endl;
        }
    }
    vis.close();
}

void s6_entrance()
{
    cout << "Step 6: Dynamic Scanning" << endl;
    s6_dynamicScan();
    s6_scanEpilogue();
    //s6_storeVisual();
    s6_releaseLink();
    s6_releaseBtTree();
    cout << "Step 6: Finished" << endl;
}

void s6_entrance(const char *ttnum, const char *snowf)
{
    cout << "Step 6: Dynamic Scanning" << endl;
    s6_dynamicScan(atoi(ttnum), string(snowf));
    s6_scanEpilogue();
    s6_storeVisual();
    s6_releaseLink();
    s6_releaseBtTree();
    cout << "Step 6: Finished" << endl;
}
