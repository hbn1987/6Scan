/****************************************************************************
 Copyright (c) 2016-2019 Robert Beverly <rbeverly@cmand.org> all rights reserved.
 ***************************************************************************/
#include "6scan.h"

uint32_t tsdiff(struct timeval *end, struct timeval *begin) {
    uint32_t diff = (end->tv_sec - begin->tv_sec) * 1000;
    diff += (end->tv_usec - begin->tv_usec) / 1000;
    return diff;
}

uint32_t tsdiffus(struct timeval *end, struct timeval *begin) {
    uint32_t diff = (end->tv_sec - begin->tv_sec) * 1000000;
    diff += (end->tv_usec - begin->tv_usec);
    return diff;
}

double now(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double)now.tv_sec + (double)now.tv_usec / 1000000.;
}

/* Sigmoid function */
double sigmoid(double t) {
    return 1.0 / (1.0 + exp(-t));
}

/*
 * Probability of taking an action, given an input time (t)
 * and range over which to decay.  Example: input milliseconds
 * with a decay from 1 to 0 over an hour timespan:
 * decayprob(t, 3600*1000)
 */
double decayprob(double t, uint32_t range) {
    t = t / (range / 12.0);
    t -= 6;
    return (1.0 - sigmoid(t));
}

double decayprob(int32_t t, uint32_t range) {
    return (decayprob((double)t, range));
}

uint8_t randuint8() {
    long val = random();
    uint8_t *p = (uint8_t *) & val;
    return *(p + 3);
}

bool checkRoot() {
    if ((getuid() != 0) && (geteuid() != 0)) {
        cerr << "Requires root!" << endl;
        exit(2);
    }
    return true;
}

double zrand() {
    static bool seeded = false;
    if (not seeded) {
        srand48((long)time(NULL));
        seeded = true;
    }
    return drand48();
}

/* generate a random libcperm key */
void permseed(uint8_t *key, uint32_t seed) {
    srand(seed);
    for (int i=0;i<KEYLEN/sizeof(int);i++) {
        int v = rand();
        memcpy(&key[(i*sizeof(int))], &v, sizeof(int));
    }
}

/* generate a random libcperm key */
void permseed(uint8_t *key) {
   permseed(key, time(NULL));
}

/* from: http://www.masaers.com/2013/10/08/Implementing-Poisson-pmf.html */
double poisson_pmf(const double k, const double lambda) {
    return exp(k * log(lambda) - lgamma(k + 1.0) - lambda);
}

/* integral log_2 */
uint32_t intlog(uint32_t in) {
    uint32_t l = 0;
    while (in >>= 1) { ++l; }
    return l;
}

/* Ensure that only one instance of 6scan is running */
void instanceLock(uint8_t instance) {
    const char *homedir = getenv("HOME");
    if (homedir) {
        char dotdir[1500];
        snprintf(dotdir, 1500, "%s/.6scan", homedir);
        struct stat st = {0};
        if (stat(dotdir, &st) == -1) {
            mkdir(dotdir, 0755);
    }
    char lockfile[1500];
    int ret = snprintf(lockfile, 1500, "%s/lock.%d", dotdir, instance);
    if (ret < 0) {
         abort();
    }
    int fd = open(lockfile, O_CREAT | O_RDWR, 0644);
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0) {
        cerr << __func__ << ": 6scan instance already running." << endl;
        exit(-1);
    }
    } else {
        cerr << __func__ << ": getenv" << endl;
        exit(-1);
    }
}

std::string padStringIfNeeded(const std::string& input) {
    int originalLength = input.length();
    int remainder = originalLength % 4;

    if (remainder != 0) {
        // 计算需要补零的个数
        int padding = 4 - remainder;

        // 在字符串前面补零
        std::string paddedString(padding, '0');
        return input + paddedString;
    } else {
        // 字符串长度已经是4的倍数，无需补零
        return input;
    }
}

string get_alias(string line, int mask) {
    int len = ceil(mask * 1.0 / 4 / 4);
    line = padStringIfNeeded(line);
    string res_str = "";
    for (int i = 0; i < len - 1; ++i)
    {
        res_str += line.substr(i * 4, 4);
        res_str += ':';
    }
    res_str += line.substr((len - 1) * 4);
    res_str += "::/";
    res_str += to_string(mask);
    return res_str;
}

// 去除字符串前后空格的函数
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    if (first == std::string::npos || last == std::string::npos) {
        // 字符串全是空格，或者是空字符串
        return "";
    }

    return str.substr(first, last - first + 1);
}