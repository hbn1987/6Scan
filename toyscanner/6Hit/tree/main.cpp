//
//  main.cpp
//  6Tree
//
//  Created by 刘知竹 on 2018/7/15.
//  Copyright © 2018年 刘知竹. All rights reserved.
//

//  功能：程序的主函数入口，以模块代码的形式保存。每个模块是能各自独立运行的部分，要运行哪个模块，
//  就将该模块的代码取消注释运行即可。

//  本代码为16进制版本

#include "definition.hpp"
//#include "m1.hpp"
#include "m2.hpp"
#include "m3.hpp"
//#include "m4.hpp"
//#include "m5.hpp"
//#include "m6.hpp"
//#include "m7.hpp"

int main(int argc, const char * argv[])
{
    // 模块1：6Tree运行的第一步，将一个保存通用表示的IPv6地址文件转换为规则的16进制地址向量式字符串，并保
    // 存到另一个文件中。
    //s1_entrance(argv[1],argv[2]);
    
    // 模块2：测试1，计算两两地址之间的改进汉明距离（即前维相似度，FDS）分布情况。
    //t1_entrance();
    
    // 模块3：包括6Tree运行的第二步和第三步。第二步，将第一步中转换好了的IPv6地址提取出来保存到地址向量数
    // 组中。第三步，从IPv6地址向量数组开始进行6Tree的树的生成（tree generation），并保存在生成树结构中，
    // 然后生成树结构的信息将以文件的形式保存下来。
    s2_entrance(argv[1]);
    s3_entrance(argv[2]);
    
    // 模块4：包括6Tree运行的第四步、第五步和第六步。第四步，根据第三步保存的生成树结构信息，构造回溯树结构，
    // 并将其中的叶子节点提取出来，生成预扫描结点数组。第五步，对预扫描结点数组执行预扫描（pre-scanning），并
    // 根据扫描结果生成链表。第六步，根据预扫描结果进行动态扫描（dynamic scanning），达到预算时停止。
    //s4_entrance();
    //s5_entrance();
    //s6_entrance(argv[2], argv[3]); // 自定义预算和雪模式文件名
    //s6_entrance();
    
    // 模块5：测试2，从包含已知所有活跃IPv6地址的测试文件中提取出一定数量的IPv6地址放入种子文件。
    //t2_entrance();
    
    // 模块6：测试3，检验实际扫描出来发现的所有活跃IPv6地址是否有重复。
    //t3_entrance();
    
    // 模块7：第七步，统计扫描结果，将不重复的不在别名区中的活跃IPv6地址保存下来。
    //s7_entrance();
    return 0;
}
