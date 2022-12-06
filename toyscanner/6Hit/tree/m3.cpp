//
//  m3.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第3个模块，包括内容如下：
//  包括6Tree运行的第二步和第三步。第二步，将第一步中转换好了的IPv6地址提取出来保存到地址向量数
//  组中。第三步，从IPv6地址向量数组开始进行6Tree的树的生成（tree generation），并保存在生成树结构中，
//  然后生成树结构的信息将以文件的形式保存下来。

#include "m2.hpp"
#include "m3.hpp"
#include "definition.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;


// 第二步开始使用的函数

void s2_getIPv6(string filename)
{
    // 从文件中提取出IPv6地址16进制字符串并保存到ipv6数组中
    ifstream infile;
    infile.open(filename);
    string ipStr;
    int num = 1;
    int tick = 1;
    while (getline(infile, ipStr))
    {
        ipv6[num++] = ipStr;
        if (num > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    ipv6_scale = num - 1;
    infile.close();
}

void s2_entrance(const char *fn)
{
    //cout << "Step 2: Input IPv6 Addresses" << endl;
    s2_getIPv6(string(fn));
    //cout << "Step 2: Finished" << endl;
}

void s2_entrance(string input)
{
    //cout << "Step 2: Input IPv6 Addresses" << endl;
    s2_getIPv6(input);
    //cout << "Step 2: Finished" << endl;
}

// 第三步开始使用的函数

void s3_strCompare(string &t, string s)
{
    // 将t与s的字符依次进行比较，如果存在不同就将t中的对应字符更改为'-'，已经变为'-'的字符不再比较
    int s_len = (int )(s.size());
    for (int i = 0; i < s_len; i++)
    {
        t[i] = t[i] == '-'? '-' : (t[i] == s[i]? t[i] : '-');
    }
}

void s3_reduce(string &ip, string t)
{
    // 从t中找出非'-'的字符，然后将ip中对应的位置去除掉
    // 例如，ip="A1234"， t="A--3-"
    // 那么reduce(ip, t)="124"
    string tmp = "";
    int t_len = (int )(t.size());
    for (int i = 0; i < t_len; i++)
    {
        tmp = t[i] == '-'? tmp + ip[i] : tmp;
    }
    ip = tmp;
}

void s3_addNode(int left, int right, struct TreeNode *node)
{
    // 向node指向的结点添加子结点
    // 更新树结构
    struct TreeNode *newNode = new struct TreeNode;
    newNode->level = node->level + 1;
    newNode->number = nodes_scale + 1;
    newNode->inf = left;
    newNode->sup = right;
    newNode->simNum = 0;
    newNode->parent = node;
    newNode->children = NULL;
    newNode->childrenNum = 0;
    int num = node->childrenNum;
    node->children[num++] = newNode;
    node->childrenNum = num;
    // 更新结点数组
    nodes[++nodes_scale] = newNode;
}

void s3_divide(int inf, int sup, int rank, int sum, struct TreeNode *node)
{
    // 根据下界inf、上界sup、划分阈值rank、结点指针node来实施划分
    // 将划分后的子结点的父节点设为node
    // sum是子结点的个数
    node->children = new struct TreeNode *[sum];
    int left;
    int right;
    left = inf;
    for (int i = inf; i < sup; i++)
    {
        if (dist[i] < rank)
        {
            right = i;
            s3_addNode(left, right, node);
            left = right + 1;
        }
    }
    right = sup;
    s3_addNode(left, sup, node);
}

void s3_treeGenerate()
{
    // 数据结构：ipv6数组和ipv6_scale变量已经建立
    // 接下来的步骤是要建立Tree、nodes以及nodes_scale数据结构
    //cout << "Generate Tree" << endl;
    // 1. 初始化树结构起始结点
    Xree.level = 1;
    Xree.number = 1;
    Xree.inf = 1;
    Xree.sup = ipv6_scale;
    Xree.simNum = 0;
    Xree.parent = NULL;
    Xree.childrenNum = 0;
    
    // 2. 将起始结点地址放入nodes的第一个位置
    nodes_scale = 1;
    nodes[nodes_scale] = &Xree;
    
    // 3. 开始以广度生成方式建立树结构
    for (int i = 1; i <= nodes_scale; i++)
    {
        struct TreeNode *node = nodes[i];
        // 3.1 归约
        // 提取该节点的inf和sup变量，然后对[inf, sup]范围内的所有地址进行分析，
        // 如果存在一些位置上的16进制字符始终保持不变，那么将这些字符约去，并保存
        // 归约信息到sim中
        int inf = node->inf;
        int sup = node->sup;
        
        // 如果该结点下拥有的地址总数为1，也就是说只有一个地址，就不再归约
        if (sup - inf == 0)
        {
            // cout << "processed nodes: " << i << " level: " << node->level << endl;
            continue;
        }
        
        string t = ipv6[inf]; // t用于记录这个地址集合中不变的字符有哪些
        for (int j = inf + 1; j <= sup; j++)
        {
            if (t.size()==33){t.erase(32, 1);}
            if (ipv6[j].size()==33){ipv6[j].erase(32, 1);}// by Hou, 去除末尾的换行符
            s3_strCompare(t, ipv6[j]);
        }
        // 此时t中的字符如果有不为'-'的，那么该就是始终保持不变的字符，将其转
        // 变为归约信息，并保存到sim中
        int t_len = (int )(t.size());
        struct simInfo sim[40];
        int simNum = 0;
        for (int j = 0; j < t_len;)
        {
            if (t[j] != '-')
            {
                int left = j;
                int right = j + 1;
                while (right < t_len)
                {
                    if (t[right] != '-')
                    {
                        right++;
                    }
                    else
                    {
                        break;
                    }
                }
                sim[simNum].str = t.substr(left, right - left);
                sim[simNum].left = left;
                sim[simNum].right = right - 1;
                simNum++;
                j = right;
            }
            else
            {
                j++;
            }
        }
        for (int j = 0; j < simNum; j++)
        {
            node->sim[j] = sim[j];
        }
        node->simNum = simNum;
        // 根据sim中的信息，对[inf, sup]范围内的数据进行归约
        for (int j = inf; j <= sup; j++)
        {
            string tmp = ipv6[j];
            s3_reduce(tmp, t);
            ipv6[j] = tmp;
        }
        
        // 3.2 划分
        
        // 如果该结点下拥有的地址总数已经小于等于LEAFSCALE（根据算法，最好设置为base，这里就是16），
        // 就将其作为叶子结点，不再划分
        if (sup - inf < ___AL_LEAFSCALE)
        {
            // cout << "processed nodes: " << i << " level: " << node->level << endl;
            continue;
        }
        
        // 对ipv6[inf, sup]范围内的地址，计算出其前相似度（FDS）分布dist[inf, sup - 1]
        for (int j = inf; j < sup; j++)
        {
            string tmp1 = ipv6[j];
            string tmp2 = ipv6[j + 1];
            int d = t1_iprdHmingDist(tmp1, tmp2, (int)(tmp2.size()));
            dist[j] = d;
        }
        
        // 新算法：直接将划分阈值设置为1
        int rank = 1; // 划分阈值始终设置为1
        int childrenNum = 0;
        for (int j = inf; j < sup; j++)
        {
            if (dist[j] < rank)
            {
                childrenNum++;
            }
        }
        childrenNum++;
        s3_divide(inf, sup, rank, childrenNum, node);
        
        // 该结点处理完毕，向界面汇报情况
        // cout << "processed nodes: " << i << " level: " << node->level << endl;
    }
    //cout << "Generate Tree Finished" << endl;
}

void s3_canGenerate(string treefname, string canfname)
{
    // 保存树的生成（tree generation）运算结果
    //cout << "Store Data Structures" << endl;
    ofstream treef;
    //ofstream canf;

    treef.open(treefname);
    //canf.open(canfname);

    // 保存树结构信息
    //cout << "Step 3.2.1: Store Tree Structure" << endl;
    int tl = nodes_scale;
    treef << "Template:" << endl;
    treef << "Location: <level, number>" << endl;
    treef << "Scale: <inf, sup>" << endl;
    treef << "SimNum: simNum" << endl;
    treef << "SimInfo:" << endl;
    treef << "i: <left, right, str>" << endl;
    treef << "..." << endl;
    treef << "Parent: parentNumber" << endl;
    treef << "ChildrenNum: childrenNum" << endl;
    treef << "Children:" << endl;
    treef << "number1 number2..." << endl;
    int tick = 1;
    for (int i = 1; i <= tl; i++)
    {
        struct TreeNode *t = nodes[i];
        treef << "Node " << i << ":" << endl;
        treef << "Location: <" << t->level << ", " << t->number << ">" << endl;
        treef << "Scale: <" << t->inf << ", " << t->sup << ">" << endl;
        treef << "SimNum: " << t->simNum << endl;
        if (t->simNum != 0)
        {
            treef << "SimInfo:" << endl;
            int k = t->simNum;
            for (int j = 0; j < k; j++)
            {
                treef << j + 1 << ": <" << t->sim[j].left << ", " << t->sim[j].right
                << ", " << t->sim[j].str << ">" << endl;
            }
        }
        if (t->parent == NULL)
        {
            treef << "Parent: " << 0 << endl;
        }
        else
        {
            treef << "Parent: " << t->parent->number << endl;
        }
        treef << "ChildrenNum: " << t->childrenNum << endl;
        int k = t->childrenNum;
        if (k != 0)
        {
            for (int j = 0; j < k; j++)
            {
                treef << t->children[j]->number << " ";
            }
            treef << endl;
        }
        if (i > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    //cout << "Step 3.2.1: Finished" << endl;
    treef.close();
    /*
    // 保存候选IPv6地址信息
    cout << "Step 3.2.2: Store Candidate IPv6 Addresses" << endl;
    canf << "Template:" << endl;
    canf << "Node: number, Level: level" << endl;
    canf << "Parent: parent_number" << endl;
    canf << "Scale: <inf, sup>" << endl;
    canf << "Expression: ABC*12** => 3 3 3 1 2 2 1 1" << endl;
    canf << "Addresses: " << endl;
    canf << "..." << endl;
    int num = 0;
    tick = 1;
    for (int i = nodes_scale; i >= 1; i--)
    {
        struct TreeNode *t = nodes[i];
        if (t->childrenNum == 0)
        {
            int inf = t->inf;
            int sup = t->sup;
            canf << "Node: " << t->number << ", Level: " << t->level << endl;
            canf << "Parent: " << t->parent->number << endl;
            canf << "Scale: <" << inf << ", " << sup << ">" << endl;
            // 从叶子结点开始，往上追溯直到起始结点，根据它们的规约信息sim，还原出原来的IPv6地址
            // 初始化表达式以及优先级数组
            string ex = ""; // 表达式
            int exg[32]; // 优先级数组，下标从0开始
            for (int j = 0; j < 32; j++)
            {
                exg[j] = 0;
            }
            int len = (int )(ipv6[t->inf].size()); // 归约出来的IPv6地址长度，后面会用来作为
            // 表达式和优先级数组的长度
            int lev = 1; // 优先级
            for (int j = 0; j < len; j++)
            {
                ex += "*";
                exg[j] = lev;
            }
            // 到这里，例如这个子结点下的归约字符串为“ABCD”等4个长度的，那么此时的表达式为“****”，优先级
            // 数组为{1, 1, 1, 1}
            // 接下来，从叶子结点开始往上回溯，每回溯到一个结点就根据其归约信息集合来更新表达式和优先级数组
            // ，仍然同上面的例子，假设一开始表达式为“****”，优先级数组为{1, 1, 1, 1}，归约信息集合为
            // {<2, 3, "ab">, <5, 5, "c">}，那么更新后形成的表达式为“**ab*c*”，优先级数组为{1, 1,
            // 2, 2, 1, 2, 1}
            do
            {
                lev++;
                int simn = t->simNum; // 归约信息个数
                string et = ""; // 新表达式
                int exgt[32]; // 新优先级数组
                for (int j = 0; j < 32; j++)
                {
                    exgt[j] = 0;
                }
                int old_tip = 0; // 原表达式、原优先级数组遍历的进度
                int new_tip = 0; // 新优先级数组、新表达式生成的进度
                for (int j = 0; j < simn; j++)
                {
                    struct simInfo simv = t->sim[j]; // 归约信息
                    int left = simv.left; // 左界
                    int right = simv.right; // 右界
                    string str = simv.str; // 归约信息字符串
                    while (new_tip < left)
                    {
                        exgt[new_tip++] = exg[old_tip];
                        et += ex[old_tip++];
                    }
                    while (new_tip <= right)
                    {
                        exgt[new_tip++] = lev;
                    }
                    et += str;
                }

                while (old_tip < len)
                {
                    exgt[new_tip++] = exg[old_tip];
                    et += ex[old_tip++];
                }
                ex = et;
                for (int j = 0; j < 32; j++)
                {
                    exg[j] = exgt[j];
                }
                // len = (int )(ex.size());
                len = new_tip;
                t = t->parent;
            }
            while (t != NULL);
            // 输出表达式和优先级数组，并还原出这个子结点下所有归约字符串原来的样子
            // 这相当于将原来的所有IPv6地址再遍历一遍，所以会是比较耗时间的
            canf << "Expression: " << ex << " => ";
            for (int j = 0; j < 32; j++)
            {
                canf << exg[j] << " ";
            }
            canf << endl;
            canf << "Addresses: " << endl;
            for (int j = inf; j <= sup; j++)
            {
                canf << ipv6[j] << endl;
                num++;
                if (num > tick * ___PR_TICK)
                {
                    cout << "processed number (*TICK): " << tick << endl;
                    tick++;
                }
            }
        }
    }
    cout << "Step 3.2.2: Finished" << endl;
    canf.close();
    */
    //cout << "Store Data Structures Finished" << endl;
}

void s3_releaseTree()
{
    // 释放生成树结构及其结点数组
    for (int i = 2; i <= nodes_scale; i++)
    {
        if (nodes[i]->children != NULL)
        {
            delete [] nodes[i]->children;
        }
        delete nodes[i];
    }
    delete [] nodes[1]->children;
}

void s3_entrance(string output)
{
    //cout << "Step 3: Generation Algorithm" << endl;
    //clock_t tick = clock();
    s3_treeGenerate();
    //clock_t tock = clock();
    //double use_time = (double )(tock - tick) / CLOCKS_PER_SEC;
    //cout << "Generation Time: " << use_time << " sec" << endl;
    s3_canGenerate(output, string(___FL_CANIPv6));
    s3_releaseTree();
    //cout << "Step 3: Finished" << endl;
}
