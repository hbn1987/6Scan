#include "6scan.h"
#include "strategy/sketch.h"

class Stats {
    public:
    int strategy = 0;
    uint64_t count = 0;       // number of probes sent
    uint64_t baddst = 0;      // checksum invalid on destination in response

    Sketch* sk = NULL;
    std::unordered_map<std::string, float> sk_map;

    uint64_t node_count = 0;

    std::vector<std::string> edgy;

    std::unordered_set<std::string> IPv4;

    struct timeval start;
    struct timeval middle;
    struct timeval end;

    Stats(int _strategy) {
        strategy = _strategy;
        gettimeofday(&start, NULL);
    };

    ~Stats() {
        if (sk)
            delete sk;
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
};
