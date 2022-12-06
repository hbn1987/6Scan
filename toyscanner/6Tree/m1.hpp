//
//  m1.hpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第1个模块，包括内容如下：
//  6Tree运行的第一步，将一个保存通用表示的IPv6地址文件转换为规则的16进制地址向量式字符串，并保
//  存到另一个文件中。

#ifndef m1_hpp
#define m1_hpp

#include <string>
#include <vector>

using namespace std;

// 实现IPv6字符串分割
vector<string> s1_split(string &s, const char &c);

string s1_DecIntToHexStr(long long num);

string s1_DecStrToHexStr(string str);

void s1_ipv6Normalize(string &s1, string &s2);

void s1_tranFile(string s1, string s2);

void s1_entrance();

#endif /* m1_hpp */
