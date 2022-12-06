//
//  m1.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第1个模块，包括内容如下：
//  6Tree运行的第一步，将一个保存通用表示的IPv6地址文件转换为规则的16进制地址向量式字符串，并保
//  存到另一个文件中。

#include "m1.hpp"
#include "definition.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

vector<string> s1_split(string &s, const char &c)
{
    // 实现IPv6字符串分割
    string buff{""};
    vector<string> v;
    
    for (auto n:s)
    {
        if (n != c)
        {
            buff += n;
        }
        else if (n == c)
        {
            v.push_back(buff);
            buff = "";
        }
    }
    if (buff != "")
    {
        v.push_back(buff);
    }
    return v;
}

string s1_DecIntToHexStr(long long num)
{
    string str;
    long long Temp = num / 16;
    int left = num % 16;
    if (Temp > 0)
        str += s1_DecIntToHexStr(Temp);
    if (left < 10)
        str += (left + '0');
    else
        str += ('a' + left - 10);
    return str;
}

string s1_DecStrToHexStr(string str)
{
    long long Dec = 0;
    for (int i = 0; i < str.size(); ++i)
    {
        Dec = Dec * 10 + str[i] - '0';
    }
    return s1_DecIntToHexStr(Dec);
}

void s1_ipv6Normalize(string &s1, string &s2)
{
    // 去除末尾的换行符
    // string t = s1.erase(s1.size() - 1, 1);
    string t = s1;
    // 进行字符串分割
    vector<string> sg = s1_split(t, ':');
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
                int cnum = k[k.size()-1]=='\r' ? (k.size()-1) : k.size();
                s2 += zero2[4 - cnum] + k;
            }
        }
    }
}

void s1_tranFile(string s1, string s2)
{
    ifstream infile;
    ofstream outfile;
    string line;
    string normalLine;
    int num = 0;
    int tick = 1;
    infile.open(s1);
    outfile.open(s2);
    while (getline(infile, line))
    {
        //cout << "Get address:" << line << endl;
        s1_ipv6Normalize(line, normalLine);
        //        检验数据转换是否正常
        //        if (normalLine.size() != 32)
        //        {
        //            cout << "find" << endl;
        //        }
        //cout << "Normalized address:" << normalLine << endl;
        outfile << normalLine << endl;
        num++;
        if (num > tick * ___PR_TICK)
        {
            cout << "processed number (*TICK): " << tick << endl;
            tick++;
        }
    }
    
    infile.close();
    outfile.close();
}

void s1_entrance()
{
    cout << "Step 1: IPv6 Normalization" << endl;
    string savePath = "./output";
    fstream f2;
 	f2.open(savePath);
	if(!f2)
	{
		mkdir(savePath.c_str(), 0777);
	}
    s1_tranFile(string(___FL_SOURCEIPV6), string(___FL_IPV6));
    //s1_tranFile("seeds", "./output/Norseeds");
    cout << "Step 1: Finished" << endl;
}
