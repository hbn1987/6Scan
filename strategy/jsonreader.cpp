/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "strategy.h"
#include <jsoncpp/json/json.h>

void readJson(string scope_file, vector<string>& prefixes) {

    Json::Reader reader;
    Json::Value root;
    Json::Value arrayObj;
    ifstream ifs(scope_file, ios::binary);

    if (scope_file.find("country") != string::npos) {
        if (reader.parse(ifs, root)) {   
            arrayObj = root["data"]["resources"]["ipv6"];
            for (auto i = 0; i < arrayObj.size(); ++i) {
                prefixes.push_back(arrayObj[i].asString());       
            }   
        } else {
            fatal("Can not prase the json file!");
        }   
    } else {
        if (reader.parse(ifs, root)) {   
            arrayObj = root["data"]["prefixes"];
            for (auto i = 0; i < arrayObj.size(); ++i) { 
                string prefix = arrayObj[i]["prefix"].asString();
                if (prefix.find("::/") != string::npos)
                    prefixes.push_back(prefix);            
            }   
        } else {
            fatal("Can not prase the json file!");
        }   
    }    
}