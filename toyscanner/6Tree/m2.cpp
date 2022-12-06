//
//  m2.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第2个模块，包括内容如下：
//  测试1，计算两两地址之间的改进汉明距离（FDS）分布情况。

#include "m2.hpp"
#include "definition.hpp"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

int t1_iprdHmingDist(string s1, string s2, int len)
{
    int num = 0;
    while (num < len && s1[num] == s2[num])
    {
        num++;
    }
    return num;
}

void t1_iHStatistics(string s3, string s4)
{
    ifstream infile;
    ofstream outfile;
    infile.open(s3);
    outfile.open(s4);
    string s1;
    string s2;
    int num = 0;
    int tick = 1;
    getline(infile, s1);
    while (getline(infile, s2))
    {
        outfile << t1_iprdHmingDist(s1, s2, 32) << endl;
        s1 = s2;
        num++;
        if (num > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    infile.close();
    outfile.close();
    infile.open(s4);
    string line;
    num = 0;
    tick = 1;
    int sta[35];
    for (int i = 0; i < 35; i++)
    {
        sta[i] = 0;
    }
    while (getline(infile, line))
    {
        int n = atoi(line.c_str());
        sta[n]++;
        num++;
        if (num > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    cout << "Statistics: " << endl;
    for (int i = 0; i <= 32; i++)
    {
        cout << i << ": " << sta[i] << endl;
    }
    infile.close();
}

void t1_entrance()
{
    cout << "Test 1: Improved Hamming Distance Distribution" << endl;
    t1_iHStatistics(string(___FL_TEST1STR), string(___FL_TEST1RES));
    cout << "Test 1: Finished" << endl;
}
