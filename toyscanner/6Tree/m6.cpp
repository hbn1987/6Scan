//
//  m6.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第6个模块，包括内容如下：
//  测试3，检验实际扫描出来发现的所有活跃IPv6地址是否有重复。

#include "m4.hpp"
#include "m6.hpp"
#include "definition.hpp"
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

void t3_resultsExamine(string findname)
{
    // 检验实际扫描的结果中是否有重复的地址
    ifstream findfile;
    string line;
    findfile.open(findname);
    long arr_scale = 0;
    string *arr;
    while (getline(findfile, line))
    {
        arr_scale++;
    }
    findfile.close();
    findfile.open(findname);
    arr = new string [arr_scale + 16];
    long sit = 0;
    while (getline(findfile, line))
    {
        arr[sit++] = line;
    }
    cout << "Read Finished" << endl;
    findfile.close();
    sort(arr, arr + arr_scale, s4_cmp);
    cout << "Sort Finished" << endl;
    for (long i = 0; i < arr_scale - 1; i++)
    {
        if (arr[i] == arr[i + 1])
        {
            cout << i << ": " << arr[i] << endl;
        }
    }
    delete [] arr;
}

void t3_entrance()
{
    cout << "Test 3: Result Examination" << endl;
    t3_resultsExamine(string(___FL_LIST));
    cout << "Test 3: Finished" << endl;
}
