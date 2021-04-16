#ifndef RANDOM_LIST_H
#define RANDOM_LIST_H

#include <stdint.h>
#include <pthread.h>
#include <vector>
#include <fstream>
#include "libcperm/cperm.h"

class IPList {
    public:
    IPList();
    virtual ~IPList() {};
    virtual uint32_t next_address(struct in_addr *in) = 0;
    virtual uint32_t next_address(struct in6_addr *in) = 0;
    virtual void read(std::istream& in) = 0;
    void read(char *in);
    virtual void seed() = 0;
    uint32_t count() { return permsize; }
    void setkey(int seed);

    uint8_t log2(uint8_t x);
    uint8_t key[KEYLEN];
    cperm_t* perm;
    uint32_t permsize;
    bool seeded;
};

class IPList4 : public IPList {
    public:
    IPList4() : IPList() {};
    virtual ~IPList4();
    uint32_t next_address(struct in_addr *in);
    uint32_t next_address_rand(struct in_addr *in);
    uint32_t next_address(struct in6_addr *in) { return 0; };
    void read(std::istream& in);
    void read_result(std::unordered_map<std::string, std::string>&);
    void seed();
    std::vector<uint32_t> targets;
};

class IPList6 : public IPList {
    public:
    IPList6() : IPList() {};
    virtual ~IPList6();
    uint32_t next_address(struct in6_addr *in);
    uint32_t next_address_rand(struct in6_addr *in);
    uint32_t next_address(struct in_addr *in) { return 0; };
    void read(std::istream& in);
    void read_hitlist(std::string in);
    void read_seedset(std::string in);
    void seed();
    void subnet6(std::string s, std::vector<struct in6_addr>& targets);
    std::vector<std::string> seeds;
    std::vector<struct in6_addr> targets;
};

#endif /* RANDOM_LIST_H */
