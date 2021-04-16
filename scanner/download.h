#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP
#include <string>

void data_download(std::string type);

std::string get_hitlist();

std::string get_seedset(std::string type);

std::string get_aliasfile();

#endif