#include "strategy.h"
#include <cmath>

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
