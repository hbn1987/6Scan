#include "strategy.h"

Sketch::Sketch(int depth, int width, int bgp_key_len, int prefix_key_len) {
    sk_.depth = depth;
    sk_.width = width;
    sk_.bgp_key_len = bgp_key_len;
    sk_.prefix_key_len = prefix_key_len;
    sk_.sum = 0;
    sk_.counts = new SBucket * [depth*width];
    for (auto i = 0; i < depth*width; ++i) {
        sk_.counts[i] = (SBucket*)calloc(1, sizeof(SBucket));
        memset(sk_.counts[i], 0, sizeof(SBucket));
    }
    sk_.ran_seeds = new unsigned long[2];
    char name[] = "Sketch";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);
    sk_.ran_seeds[0] = GenHashSeed(seed);
    sk_.ran_seeds[1] = GenHashSeed(++seed);
}

Sketch::~Sketch() {
    for (auto i = 0; i < sk_.depth*sk_.width; i++) {
        free(sk_.counts[i]);
    }
    delete [] sk_.ran_seeds;
    delete [] sk_.counts;
}

void Sketch::Update(std::string key) {
    sk_.sum ++;
    unsigned long row, column;
    row = MurmurHash64A((unsigned char*)key.c_str(), sk_.bgp_key_len, sk_.ran_seeds[0]) % sk_.depth;
    column = MurmurHash64A((unsigned char*)key.c_str(), sk_.prefix_key_len, sk_.ran_seeds[1]) % sk_.width;
    int index = row*sk_.width+column;
    Sketch::SBucket *sbucket = sk_.counts[index];
    sbucket->sum ++;
    std::string prefix_key = key.substr(0, sk_.prefix_key_len);
    if (sbucket->key.empty()) {
        sbucket->key = prefix_key;
        sbucket->count = 1;
    } else if(sbucket->key == prefix_key) {
        sbucket->count ++;
    } else {
        sbucket->count --;
        if (sbucket->count < 0) {
            sbucket->key = prefix_key;
            sbucket->count = -sbucket->count;
        }
    }
}

void Sketch::PrintAll() {
    for (auto i = 0; i < sk_.depth*sk_.width; ++i)
    {
        int est = (sk_.counts[i]->sum + sk_.counts[i]->count)/2;
        std::cout <<"Bucket " << i << ": " << sk_.counts[i]->key << ", " << est << std::endl;
    }
}

void Sketch::Getsk(std::unordered_map<std::string, float>& sk_map)
{
    int total_count = 0;
    for (auto i = 0; i < sk_.depth*sk_.width; ++i) {
        total_count += sk_.counts[i]->count;
    }
    int tmp_sum = sk_.sum;
    for (auto i = 0; i < sk_.depth*sk_.width; ++i) {
        float reward = (sk_.counts[i]->sum + sk_.counts[i]->count)*1.0/(tmp_sum + total_count);
        if (reward > Alias_Threshold) {
            sk_.counts[i]->count = 0;
            sk_.sum -= sk_.counts[i]->sum;
            sk_.counts[i]->sum = 0;
            std::cout << "Alias alert: " << sk_.counts[i]->key << std::endl;
        }
    }
    for (auto i = 0; i < sk_.depth*sk_.width; ++i) {
        if (sk_.counts[i]->sum) {
            float reward = (sk_.counts[i]->sum + sk_.counts[i]->count)*1.0/(sk_.sum + total_count);
            PAIR tmp(sk_.counts[i]->key, reward);
            sk_map.insert(tmp);
        }
    }
}