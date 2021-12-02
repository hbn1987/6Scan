/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "6scan.h"
#include "strategy/tree.h"

class Stats {
    public:
    int strategy = 0;
    int mask;
    uint64_t count = 0;       // number of probes sent
    uint64_t baddst = 0;      // checksum invalid on destination in response

    Node_List nodelist;

    std::unordered_map<std::string, int> prefix_map;
    std::vector<std::string> prefixes;

    std::unordered_set<std::string> IPv4;

    struct timeval start;
    struct timeval middle;
    struct timeval end;

    Stats(int _strategy) {
        strategy = _strategy;
        gettimeofday(&start, NULL);
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

    void dump_alias(FILE *alias_out, std::string alias_prefix) {
        fprintf(alias_out, "%s\n", alias_prefix.c_str());
    }
};
