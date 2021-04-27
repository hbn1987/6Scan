#include "6scan.h"
#include <cmath>

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

// Decimal to Hexadecimal
string dec2hex(int dec, int len)
{
    string subnet_hex(len, '0');
    len--;
    int m;
    while (dec > 0)
    {
        m = dec % 16;
        dec /= 16;
        if (m > 9)
        subnet_hex[len--] = 'a' + m - 10;
        else
        subnet_hex[len--] = '0' + m;
    }
    while (len >= 0)
    {
        subnet_hex[len--] = '0';
    }
    return subnet_hex;
}

string vec2colon(string line)
{
    // Translate the base-16 mode notation into the colon-hexadecimal notation.
    string res_str = "";
    for (int i = 0; i < 7; i++)
    {
        res_str += line[i * 4 + 0];
        res_str += line[i * 4 + 1];
        res_str += line[i * 4 + 2];
        res_str += line[i * 4 + 3];
        res_str += ':';
    }
    res_str += line[28];
    res_str += line[29];
    res_str += line[30];
    res_str += line[31];
    return res_str;
}

string get_scan_time() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    return to_string(1900 + ltm->tm_year) + to_string(1 + ltm->tm_mon) + to_string(ltm->tm_mday);
}

vector<string> str_split(string &s, const char &c) {
    // Split the string.
    string buff{""};
    vector<string> v;

    for (auto n : s)
    {
        if (n != c)
            buff += n;
        else if (n == c)
        {
            v.push_back(buff);
            buff = "";
        }
    }
    if (buff != "")
        v.push_back(buff);
    return v;
}

string seed2vec(string line) {
    // Translate the colon-hexadecimal notation into the base-16 mode notation.
    if (line[line.length()-1]==':' && line[line.length()-2]!=':')
    {
        line = line + ":";
    }
    string vec;
    vector<string> segment = str_split(line, ':');
    int segment_num = segment.size();
    for (auto i = 0; i < segment_num; ++i)
    {
        if (segment[i].length())
        {
            int add_zero = 4 - segment[i].length();
            if (add_zero)
            {
                string add_string(add_zero, '0');
                segment[i] = add_string + segment[i];
            }
        }
        else if (segment[i].length() == 0 && i != segment_num)
        {
            int add_seg = 8 - segment_num + 1;
            string add_zero(add_seg * 4, '0');
            segment[i] = add_zero;
        }
        else
        {
            string add_zero (4, '0');
            segment[i] = add_zero;
        }
        vec += segment[i];
    }
    return vec;
}

int str_cmp(string s1, string s2)
{
    return s1.compare(s2) < 0;
}

string ipv4_check(string ip) {
    string ipv4 = "";
    int flag = 0;
    for (auto i = 0; i < ip.size(); ++i) {
        if (isdigit(ip[i]) == 0 && ip[i] != ':')
            return "";
        if (ip[i] == ':') {
            string tmp = ip.substr(flag, i-flag);
            if (atoi(tmp.c_str()) > 255)
                return "";
            else {
                flag = i + 1;
                ipv4 = ipv4 + tmp + ".";
            }
        }
    }
    if (atoi(ip.substr(flag).c_str()) > 255) {
        return "";
    } else {
        ipv4 += ip.substr(flag);
    }
    return ipv4;
}

string get_ipv4(string ipv6) {
    string::size_type idx1, idx2;
    for (auto i = 0; i < 4; ++i) {
        idx1 = ipv6.find_last_of(":", i == 0 ? ipv6.size() - 1 : idx1 - 1);
        if (idx1 == string::npos) {
            return "";
        }
    }

    idx2 = ipv6.substr(idx1 + 1).find("::");
    if (idx2 != string::npos)
        return "";
    else
        return ipv4_check(ipv6.substr(idx1 + 1));
}
