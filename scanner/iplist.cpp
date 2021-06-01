#include "6scan.h"

static uint32_t NETMASKS[] = {
    0xffffffff, 0x80000000, 0xc0000000, 0xe0000000,
    0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000,
    0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
    0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000,
    0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000,
    0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
    0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
    0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe
};

IPList::IPList() : seeded(false) {
    perm = NULL;
    permsize = 0;
    memset(key, 0, KEYLEN);
}

void IPList::setkey(int seed) {
    permseed(key, seed);
}

IPList4::~IPList4() {
    targets.clear();
    cperm_destroy(perm);
}

IPList6::~IPList6() {
    targets.clear();
    seeds.clear();
    cperm_destroy(perm);
}

/* seed */
void IPList4::seed() {
    PermMode mode = PERM_MODE_CYCLE;
    assert(targets.size() > 0);
    permsize = targets.size();
    if (permsize < 1000000)
        mode = PERM_MODE_PREFIX;
    perm = cperm_create(permsize, mode, PERM_CIPHER_RC5, key, 16);
    assert(perm);
    seeded = true;
}

void IPList6::seed() {
    PermMode mode = PERM_MODE_PREFIX;
    assert(targets.size() > 0);
    permsize = targets.size();
    if (permsize > 5000000) {
    mode = PERM_MODE_CYCLE;
    //cout << ">> Warning: reduced IPv6 performance with this many targets" <<  endl;
    }
    perm = cperm_create(permsize, mode, PERM_CIPHER_SPECK, key, 8);
    assert(perm);
    seeded = true;
}

/* Read list of input IPs */
void IPList::read(char *in) {
    if (*in == '-') {
    read(cin);
    } else {
    ifstream ifile(in);
    if (ifile.good() == false)
        fatal("Bad input file: %s", in);
    read(ifile);
    }
}

/* Read IPv4 addresses */
void IPList4::read(istream& inlist) {
    string line;
    struct in_addr addr;
    while (getline(inlist, line)) {
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase( remove(line.begin(), line.end(), '\r'), line.end() );
    if (inet_aton(line.c_str(), &addr) != 1)
        fatal("*** Couldn't parse IPv4 address: %s", line.c_str());
    targets.push_back(addr.s_addr);
    }
    cout << "IPv4 targets: " << targets.size() << endl;
}

void IPList4::read_result(unordered_map<string, string>& results) {
    string line;
    struct in_addr addr;
    for (auto iter : results) {
        if (isdigit(iter.second[0]) != 0) {
            line = iter.second;
            if (inet_aton(line.c_str(), &addr) != 1) {
                warn("*** Couldn't parse IPv4 address: %s", line.c_str());
                continue;
            }
            targets.push_back(addr.s_addr);
        }
    }
    cout << "IPv4 targets: " << targets.size() << endl;
}

void IPList6::subnet6(string s, vector<struct in6_addr>& targets) {
    struct in6_addr start;
    struct in6_addr next;
    uint64_t cnt;

    uint8_t m;
    char *p = strdup(s.c_str());
    if (2 == sscanf(s.c_str(), "%[a-fA-F0-9:]/%hhu", p, &m)) {
        //cout << "Got IPv6 address: " << p << " prefix length: " << int (m) << endl;
        if (inet_pton(AF_INET6, p, &start) != 1) {
            fatal("Error parsing IPv6 address: %s", p);
        }
        if (m < 96) {
            fatal("IPv6 Prefix must be /96 or larger!");
        }
        if (m == 128)
            cnt = 0;
        else
            cnt = (1 << (128 - m)) - 2;

        uint8_t mask = m % 32;
        //char output[INET6_ADDRSTRLEN];
        //printf("MASK: %08x\n", NETMASKS[mask]);
        if (m == 128)
            start.s6_addr32[3] = htonl((ntohl(start.s6_addr32[3]) & NETMASKS[mask]));
        else
            start.s6_addr32[3] = htonl((ntohl(start.s6_addr32[3]) & NETMASKS[mask])+1);
        targets.push_back(start);
        //inet_ntop(AF_INET6, &start, output, INET6_ADDRSTRLEN);
        //cout << "Start: " << output << endl;

        memcpy(&next, &start, sizeof(struct in6_addr));
        for (auto i = 1; i <= cnt; ++i)
        {
            next.s6_addr32[3] = start.s6_addr32[3] + htonl(i);
            targets.push_back(next);
        }
        //inet_ntop(AF_INET6, &next, output, INET6_ADDRSTRLEN);
        //cout << "End: " << output << endl;
    } else {
        fatal("Error parsing IPv6 subnet: %s", s.c_str());
    }
    free(p);
}

/* Read list of input IPs */
void IPList6::read(istream& inlist) {
    string line;
    struct in6_addr addr;
    while (getline(inlist, line)) {
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase( remove(line.begin(), line.end(), '\r'), line.end() );
    subnet6(line, targets);
    }
    cout << "IPv6 targets: " << targets.size() << endl;
}

void IPList6::read_hitlist(string in) {
    ifstream inlist;
    inlist.open(in);
    string line;
    struct in6_addr addr;
    getline(inlist, line); // Skip the description line
    while (getline(inlist, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase( remove(line.begin(), line.end(), '\r'), line.end() );
        line = line + string("/128");
        subnet6(line, targets);
    }
    inlist.close();
    cout << "The size of hitlist: " << targets.size() << endl;
}

void IPList6::read_seedset(string in) {
    ifstream inlist;
    inlist.open(in);
    string line;
    while (getline(inlist, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase( remove(line.begin(), line.end(), '\r'), line.end());
        seeds.push_back(seed2vec(line));
    }
    inlist.close();
    //cout << "The size of seedset: " << seeds.size() << endl;
}

uint32_t IPList4::next_address(struct in_addr *in) {
    return next_address_rand(in);
}

/* random next address */
uint32_t IPList4::next_address_rand(struct in_addr *in) {
    static uint32_t next = 0;

    if (not seeded)
        seed();

    if (PERM_END == cperm_next(perm, &next))
        return 0;

    in->s_addr = targets[next];
    return 1;
}

uint32_t IPList6::next_address(struct in6_addr *in) {
    return next_address_rand(in);
}

/* random next address */
uint32_t IPList6::next_address_rand(struct in6_addr *in)
{
    static uint32_t next = 0;
    if (not seeded)
        seed();

    if (PERM_END == cperm_next(perm, &next))
        return 0;
    *in = targets[next];
    return 1;
}
