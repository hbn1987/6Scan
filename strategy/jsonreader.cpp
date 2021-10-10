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

void hitlist_analysis(string seedset, string region_limit) {
    vector<string> countries, prefixes;
    get_countryfile_all(countries);

    Patricia *tree = new Patricia(128);    

    vector<pair<string, int>> counter;
    int index = 0;

    /* Build the region tree */
    for (auto& country : countries) {
        readJson(country, prefixes);
        string region = country.substr(19, 2);
        counter.push_back(make_pair<string, int>(region.c_str(), 0));
        for (auto& it : prefixes){            
            tree->add(AF_INET6, it.c_str(), index);
        }
        index++;
    }

    string line;
    int line_count = 0;
    unordered_map<string, set<string>> prefix_map, region_prefix_map;
    ifstream infile;
    infile.open(seedset);
    while (getline(infile, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        int* result = (int*) tree->get(AF_INET6, line.c_str());
        if (result) {
            counter[*result].second++;
            prefix_map[counter[*result].first].insert(seed2vec(line).substr(0, 8));
        }            
        line_count++;
    }
    infile.close();
    cout << "Gasser's Hitlist: " << endl;
    for (auto i = 0; i < counter.size(); ++i) {
        cout << "Region: " << counter[i].first << ", Number: " << counter[i].second << ", Ratio: " \
        << counter[i].second * 1.0 / line_count << ", Number of /32 prefixes: " << prefix_map[counter[i].first].size() << endl;
    }    
    delete tree;

    vector<string> hitlists;
    get_region_hitlist_all(hitlists);
    /* Analyze the number of prefixes in each country */
    for (auto& hitlist : hitlists) {
        string region = hitlist.substr(25, 2);
        infile.open(hitlist);
        while (getline(infile, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(remove(line.begin(), line.end(), '\r'), line.end());
            region_prefix_map[region].insert(seed2vec(line).substr(0, 8));
            prefix_map[region].insert(seed2vec(line).substr(0, 8));          
        }
        infile.close();
    }
    cout << "Our region-level seedsets: " << endl;
    for (auto iter = region_prefix_map.begin(); iter != region_prefix_map.end(); ++iter) {
        cout << "Region: " << iter->first << ", Number of /32 prefixes: " << iter->second.size() << endl;
    }
    cout << "Aggregate prefix statistics: " << endl;
    for (auto iter = prefix_map.begin(); iter != prefix_map.end(); ++iter) {
        cout << "Region: " << iter->first << ", Number of /32 prefixes: " << iter->second.size() << endl;
    }   
}

void alias_analysis(string alias_file) {
    vector<string> aliases;
    get_aliasfile_all(aliases);

    Patricia *tree = new Patricia(128);     

    /* Build Gasser's alias_prefix tree */
    ifstream infile;
    string Gasser_alias = get_aliasfile();
    infile.open(Gasser_alias);
    tree->populate6(infile);
    infile.close();
 
    unordered_map<string, string> prefix_region;
    unordered_map<string, pair<int, int>> region_counter;
    string line;
    for (auto& alias_file : aliases) {
        string region = alias_file.substr(23, 2);
        region_counter.insert({region, make_pair<int, int>(0, 0)});
        infile.open(alias_file);
        while (getline(infile, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase( remove(line.begin(), line.end(), '\r'), line.end());
            line = line.substr(0, line.find("/")) + line.substr(line.length() - 2);
            prefix_region.insert({line, region});
        }
        infile.close();
    }

    for (auto& it : prefix_region) {
        int* result = (int*) tree->get(AF_INET6, it.first.c_str());
        if (NULL == result)           
            region_counter[it.second].second++;
        else
            region_counter[it.second].first++;
    }

    for (auto& it : region_counter) {
        cout << "Region: " << it.first << ", Number of coincidence prefixes: " << it.second.first << ", Newly discovered prefixes: " << it.second.second << endl;
    }    
    delete tree;
}

void hitlist_region_seeds(string seedset, string region_limit, string type_str) {
    vector<string> prefixes;
    string country = get_countryfile(region_limit);

    Patricia *tree = new Patricia(128);    

    int country_code = 1;

    /* Build the region tree */
    readJson(country, prefixes);
    for (auto& it : prefixes){            
        tree->add(AF_INET6, it.c_str(), country_code);
    }

    string outfile = "./output/Gasser_" + region_limit + "_" + seedset.substr(15);
    ofstream file_writer(outfile, ios_base::out);
    unordered_set<string> country_seed;
    
    string line;
    ifstream infile;
    infile.open(seedset);
    while (getline(infile, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        int* result = (int*) tree->get(AF_INET6, line.c_str());
        if (result) {
            country_seed.insert(line);
            file_writer << line << "\n";
        }            
    }
    infile.close();
    file_writer.close();
    delete tree;

    country = get_region_hitlist(region_limit, type_str);

    infile.open(country);
    while (getline(infile, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        country_seed.insert(line);
    }
    infile.close();

    outfile = "./output/Mixed_" + region_limit + "_" + seedset.substr(15);
    ofstream file_writer1(outfile, ios_base::out);
    for (auto& iter : country_seed){
        file_writer1 << iter << "\n";
    }
    file_writer1.close();
}