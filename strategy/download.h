/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include <string>

void data_download(std::string type);

std::string get_hitlist();

std::string get_seedset(std::string type, char* region);

std::string get_seedset(std::string type);

std::string get_aliasfile();

std::string get_countryfile(std::string scope);

void get_countryfile_all(std::vector<std::string>& countries);

std::string get_asfile(std::string scope);

void get_aliasfile_all(std::vector<std::string>& aliases);

std::string get_region_hitlist(std::string scope, std::string type_str);

void get_region_hitlist_all(std::vector<std::string>& hitlists);
