/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "6scan.h"
#include "strategy/tree.h"

#include <array>
#include <numeric>
#include <string>
#include <sstream>
class FixedSizeHashMap {
public:
    FixedSizeHashMap(size_t size) : size_(size), data_(size, std::make_pair("", std::array<int, 16>{})) {}

    void initInsert(const std::string& key) {        
        size_t index = hashFunction(key) % size_;
        data_[index] = std::make_pair(key, std::array<int, 16>{});
    }

    void insert(const std::string& key, const std::string& hexString) {
        int digit = std::stoi(hexString, nullptr, 16);
        
        size_t index = hashFunction(key) % size_;
        if (data_[index].first == key) {
            data_[index].second[digit]++;
        }            
    }

    std::vector<std::string> traverse(int threshold) {
        std::vector<std::string> alias_candidates;

        for (auto& pair : data_) {
            int sum = std::accumulate(pair.second.begin(), pair.second.end(), 0);
            if (sum >= threshold) {
                alias_candidates.push_back(pair.first);
                pair.second = std::array<int, 16>{};  // Mark for deletion
            }
        }

        // Remove marked elements using std::remove_if; Radical deletion of possible alias prefixes
        data_.erase(std::remove_if(data_.begin(), data_.end(),
                                   [](const auto& pair) { return pair.second == std::array<int, 16>{}; }),
                    data_.end());

        return alias_candidates;
    }
    
    std::vector<std::string> acquiredPrefixes() const {
        std::string prefix;
        std::vector<std::string> Prefixes;
        std::stringstream ss;

        for (const auto& pair : data_) {
            for (auto i = 0; i < 16; ++i) {
                if (pair.second[i] > 0 and !pair.first.empty()) {
                    ss << std::hex << i;
                    std::string hexString = ss.str();
                    prefix = pair.first + ss.str();
                    Prefixes.push_back(prefix);
                    ss.str("");  // Clear the stringstream
                }
            }
        }

        return Prefixes;
    }
    
    std::vector<std::string> nonEmptyKeys() const {
        std::vector<std::string> keys;
        for (const auto& pair : data_) {
            if (!pair.first.empty()) {
                keys.push_back(pair.first);
            }
        }
        return keys;
    }

    size_t nonEmptyKeyCount() const {
        return std::count_if(data_.begin(), data_.end(),
                             [](const auto& pair) { return !pair.first.empty(); });
    }

    void clear() {
        data_.clear();
        data_.resize(size_, std::make_pair("", std::array<int, 16>{}));
    }

    //for debug
    size_t getDataSize() const {
        size_t vectorSize = sizeof(data_);        
        for (const auto& pair : data_) {
            vectorSize += sizeof(pair);
            vectorSize += sizeof(std::string) + pair.first.capacity(); // Include string's dynamic memory
            vectorSize += sizeof(std::array<int, 16>); // Include std::array's size
        }
        return vectorSize;
    }   
    
    //for debug
    std::array<int, 16> find(const std::string& key) const {
        size_t index = hashFunction(key) % size_;
        if (data_[index].first == key) {
            return data_[index].second;
        } else 
            return {}; 
    }

    // for debug
    void printArray(const std::array<int, 16>& value) const {
        std::cout << "Array elements: ";
        for (int v : value) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }

private:
    //std::hash
    size_t hashFunction(const std::string& key) const {
        return std::hash<std::string>{}(key);
    }

    size_t size_;
    std::vector<std::pair<std::string, std::array<int, 16>>> data_;
};

class Stats {
    public:
    int strategy = 0;
    int mask = 32;
    uint64_t count = 0;       // number of probes sent
    uint64_t baddst = 0;      // checksum invalid on destination in response
    std::string addrtype = "low";

    Node_List nodelist;

    // std::unordered_map<std::string, int> prefix_map;
    // std::vector<std::string> prefixes;
    FixedSizeHashMap hashMap{100000}; // 100K prefixes at most by default

    std::unordered_set<std::string> IPv4;

    struct timeval start;
    struct timeval middle;
    struct timeval end;

    std::vector<int> budgets_list;

    Stats(ScanConfig * config) {
        strategy = config->strategy;
        if (config->addrtype)
            addrtype = std::string(config->addrtype);
        gettimeofday(&start, NULL);
        const int n = 10;
        int budgets[n] = {10, 50, 100, 500, 1000, 1500, 2000, 2500, 3000, 4000};
        int unit = 1000000;        
        for (int i = 0; i < n; ++i)
            budgets_list.push_back(budgets[i] * unit);
    };

    ~Stats() {
        if (nodelist.size())
            nodelist.clear();
    };

    void prepare_time() {
        gettimeofday(&middle, NULL);
    };

    void end_time() {
        gettimeofday(&end, NULL);
    };

    void dump(FILE *out) {
        float t = (float) tsdiff(&end, &start) / 1000.0;
        float m = (float) tsdiff(&middle, &start) / 1000.0;
        fprintf(out, "# Budget: %" PRId64 "\n", count);
        fprintf(out, "# Probing rate: %2.2fpps\n", (float) count / t);
        fprintf(out, "# Time cost: Total %2.2fs, Preparation %2.2fs, Scanning %2.2fs\n", t, m, t-m);

        fprintf(stdout, "# Budget: %" PRId64 "\n", count);
        fprintf(stdout, "# Probing rate: %2.2fpps\n", (float) count / t);
        fprintf(stdout, "# Time cost: Total %2.2fs, Preparation %2.2fs, Scanning %2.2fs\n", t, m, t-m);
    };

    void dumpHeuristic(FILE *out) {
        float t = (float) tsdiff(&end, &start) / 1000.0;
        fprintf(out, "# Budget: %" PRId64 "\n", count);
        fprintf(out, "# Probing rate: %2.2fpps\n", (float) count / t);
        fprintf(out, "# Time cost: Total %2.2fs\n", t);

        fprintf(stdout, "# Budget: %" PRId64 "\n", count);
        fprintf(stdout, "# Probing rate: %2.2fpps\n", (float) count / t);
        fprintf(stdout, "# Time cost: Total %2.2fs\n", t);
    };

    void dump_budget(FILE *out) {
        fprintf(out, "# Budget consumption: %" PRId64 "\n", count);
    }

    void dump_alias(FILE *alias_out, std::string alias_prefix) {
        fprintf(alias_out, "%s\n", alias_prefix.c_str());
    }

    void dump_space(FILE *space_out, std::string space, std::string hc) {
        fprintf(space_out, "%s, %s\n", space.c_str(), hc.c_str());
    }

    void erase_budget() {
        std::vector<int>::iterator k = budgets_list.begin();
        budgets_list.erase(k);
    }
};
