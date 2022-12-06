//
//  m7.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第7个模块，包括内容如下：
//  第七步，统计扫描结果，将不重复的不在别名区中的活跃IPv6地址保存下来。

#include "m4.hpp"
#include "m7.hpp"
#include "definition.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

int s7_relation(string acip, string ali)
{
    // 判断活跃地址字符串与别名区表达式之间的关系
    for (int i = 0; i < 32; i++)
    {
        if (ali[i] != '*')
        {
            if (acip[i] < ali[i])
            {
                return ___AL_ALI_BL;
            }
            else if (acip[i] > ali[i])
            {
                return ___AL_ALI_UP;
            }
        }
    }
    return ___AL_ALI_IN;
}

void s7_statistic(string listname, string aliasname, string flistname)
{
    // 从listname指向的文件中取出所有的地址，并进行去重，报告重复地址的数量
    // 理论上讲重复地址数量应该是为0的，但是这里出现了少量的重复地址，然后将
    // 去重后的排好序的地址数组与排好序的别名区数组（从aliasname指向的文件中
    // 取出）进行比对，剔除位于别名区中的数组
    ifstream listfile;
    string line;
    listfile.open(listname);
    long arr_scale = 0;
    string *arr;
    while (getline(listfile, line))
    {
        arr_scale++;
    }
    listfile.close();
    listfile.open(listname);
    arr = new string [arr_scale + 16];
    long sit = 0;
    while (getline(listfile, line))
    {
        arr[sit++] = line;
    }
    cout << "Read Finished" << endl;
    listfile.close();
    sort(arr, arr + arr_scale, s4_cmp);
    cout << "Sort Finished" << endl;
    string *flist = new string [arr_scale + 16];
    long flist_scale = 0;
    for (long i = 0; i < arr_scale - 1; i++)
    {
        if (arr[i] != arr[i + 1])
        {
            flist[flist_scale++] = arr[i];
        }
    }
    flist[flist_scale++] = arr[arr_scale - 1];
    delete [] arr;
    cout << "Repeated Address Number: " << arr_scale - flist_scale << endl;
    // 取出别名区信息并将别名区排序，然后进行比对剔除，最后将f剩下的地址保存到flist文件中
    string *ali;
    long ali_scale = 0;
    ifstream alifile;
    alifile.open(aliasname);
    while (getline(alifile, line))
    {
        if (line[0] != 'A')
        {
            ali_scale++;
        }
    }
    alifile.close();
    ali = new string [ali_scale + 16];
    alifile.open(aliasname);
    sit = 0;
    while (getline(alifile, line))
    {
        if (line[0] != 'A')
        {
            ali[sit++] = line;
        }
    }
    alifile.close();
    sort(ali, ali + ali_scale, s4_cmp);
    string *res = new string [arr_scale + 16];
    long res_scale = 0;
    long ali_sit = 0;
    long tick = 1;
    for (long i = 0; i < flist_scale; i++)
    {
        // 对于flist[i]根据情况分别进行判断，如果位于两个别名区之间，那么输入到res数组中
        // 如果位于别名区内，那么不输入到res数组中
        int sta = s7_relation(flist[i], ali[ali_sit]);
        if (sta == ___AL_ALI_IN)
        {
            continue;
        }
        else if (sta == ___AL_ALI_BL)
        {
            do
            {
                ali_sit--;
                if (ali_sit < 0)
                {
                    break;
                }
                sta = s7_relation(flist[i], ali[ali_sit]);
            } while (sta == ___AL_ALI_BL);
            if (ali_sit < 0)
            {
                ali_sit = 0;
                res[res_scale++] = flist[i];
            }
            else if (sta == ___AL_ALI_UP)
            {
                res[res_scale++] = flist[i];
            }
        }
        else // sta == ___AL_ALI_UP
        {
            do
            {
                ali_sit++;
                if (ali_sit >= ali_scale)
                {
                    break;
                }
                sta = s7_relation(flist[i], ali[ali_sit]);
            } while (sta == ___AL_ALI_UP);
            if (ali_sit >= ali_scale)
            {
                ali_sit = ali_scale - 1;
                res[res_scale++] = flist[i];
            }
            else if (sta == ___AL_ALI_BL)
            {
                res[res_scale++] = flist[i];
            }
        }
        if (i > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    delete [] flist;
    delete [] ali;
    ofstream resfile;
    resfile.open(flistname);
    for (long i = 0; i < res_scale; i++)
    {
        resfile << res[i] << endl;
    }
    resfile.close();
    delete [] res;
}

void s7_entrance()
{
    cout << "Step 7: Statistics" << endl;
    s7_statistic(string(___FL_LIST), string(___FL_ALIAS), string(___FL_FLIST));
    cout << "Step 7: Finished" << endl;
}
