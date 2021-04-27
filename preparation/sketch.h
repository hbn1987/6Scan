#ifndef SKETCH_H
#define SKETCH_H

#include <vector>
#include <unordered_map>
#include <utility>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include "hash.h"

#define Alias_Threshold 0.4

class Sketch {

    typedef struct SBUCKET_type {
        int sum = 0;
        int count = 0;
        std::string key = NULL;
    } SBucket;

    struct SK_type {
        //Total buckets
        int sum;
        //Bucket table
        SBucket **counts;
        //Sketch depth and width
        int depth;
        int width;
        //Key word bits
        int bgp_key_len;
        int prefix_key_len;
        //Random seeds
        unsigned long  *ran_seeds;
    } sk_;

    public:
    Sketch(int depth, int width, int bgp_key_len, int prefix_key_len);

    ~Sketch();

    void Update(std::string key);

    void PrintAll();

    void UpdateReward();

    void Getsk(std::unordered_map<std::string, float>& sk_map);
};

typedef std::pair<std::string, float> PAIR;

#endif
