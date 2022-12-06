//
//  m7.hpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/16.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  第7个模块，包括内容如下：
//  第七步，统计扫描结果，将不重复的不在别名区中的活跃IPv6地址保存下来。

#ifndef m7_hpp
#define m7_hpp

#include <string>

using namespace std;

int s7_relation(string acip, string ali);

void s7_statistic(string listname, string aliasname, string flistname);

void s7_entrance();

#endif /* m7_hpp */
