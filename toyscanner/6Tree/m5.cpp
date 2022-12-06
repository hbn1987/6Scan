//
//  m5.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第5个模块，包括内容如下：
//  测试2，从包含已知所有活跃IPv6地址的测试文件中提取出一定数量的IPv6地址放入种子文件。

#include "m4.hpp"
#include "m5.hpp"
#include "definition.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

void t2_ranSelectAddrs(string tfilename, string ifilename, int num)
{
    // 从测试IPv6地址中选取一定数量的地址放入种子文件中，测试IPv6地址要求已经整理为了
    // 16进制字符串格式
    ifstream infile;
    ofstream outfile;
    string line;
    infile.open(tfilename);
    outfile.open(ifilename);
    string *testAddrs = new string [___AL_TIPNUM];
    int testAddrs_scale = 0;
    
    while (getline(infile, line))
    {
        testAddrs[testAddrs_scale++] = line;
    }
    
    s5_RandomGenerator sg;
    srand((int )time(0));
    random_shuffle(testAddrs, testAddrs + testAddrs_scale - 1, sg);
    sort(testAddrs, testAddrs + num, s4_cmp);
    
    for (int i = 0; i < num; i++)
    {
        outfile << testAddrs[i] << endl;
    }
    
    infile.close();
    outfile.close();
    delete [] testAddrs;
}

void t2_entrance()
{
    cout << "Test 2: Randomly Select IPv6 Addresses" << endl;
    t2_ranSelectAddrs(string(___FL_TEST), string(___FL_IPV6), 900000);
    cout << "Test 2: Finished" << endl;
}
