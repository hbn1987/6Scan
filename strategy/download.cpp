#include "strategy.h"
#include <curl/curl.h>
#include <curl/easy.h>

bool download_raw_data(const char* filename, string url) {
    CURL* curl;
    CURLcode res;
    FILE* fp = fopen(filename, "w");
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: Agent-007");
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if (res != 0) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        fclose(fp);
        return true;
    }
    return false;
}

vector<string> get_file_names(string dir_name)
{
	vector<string> files;
	DIR *dir = opendir(dir_name.c_str());
	struct dirent *ptr;
	while ((ptr=readdir(dir)) != NULL)
	{
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
            continue;
	    files.push_back(ptr->d_name);
	}
	closedir(dir);
	return files;
}

void data_download(string type)
{
    if (0 != access(string(DOWNLOAD).c_str(),0))
        mkdir(string(DOWNLOAD).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    string input;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string querytime = to_string(1900 + ltm->tm_year) + to_string(1 + ltm->tm_mon) + to_string(ltm->tm_mday);
    const char* filename;
    string filename_str, url, cmd;
    vector<string> file_names = get_file_names(DOWNLOAD);
    vector<string>::iterator it;

    if (type == "hitlist") {
        filename_str = string("hitlist_") + querytime;
        it = find(file_names.begin(), file_names.end(), filename_str);
        if (it == file_names.end()) {
            cout << "Start downloading the hitlist" << endl;
            filename_str= DOWNLOAD + string("/") + filename_str + string(".xz");
            filename = filename_str.c_str();
            url = "https://alcatraz.net.in.tum.de/ipv6-hitlist-service/open/responsive-addresses.txt.xz";
            download_raw_data(filename, url);
            cout << "Finish downloading the hitlist" << endl;
            cmd = "xz -d ./download/hitlist_" + querytime + string(".xz");
            int sysret = system(cmd.c_str());
            if (sysret == -1)
                cout << "File decompression or download failed" << endl;
        } else
            cout << "The hitlist has already been downloaded today" << endl;
    } else if (type == "alias") {
        filename_str = string("aliased_prefixes_") + querytime;
        it = find(file_names.begin(), file_names.end(), filename_str);
        if (it == file_names.end()) {
            cout << "Start downloading the aliased prefixes" << endl;
            filename_str= DOWNLOAD + string("/") + filename_str + string(".xz");
            filename = filename_str.c_str();
            url = "https://alcatraz.net.in.tum.de/ipv6-hitlist-service/open/aliased-prefixes.txt.xz";
            download_raw_data(filename, url);
            cout << "Finish downloading the aliased prefixes" << endl;
            cmd = "xz -d ./download/aliased_prefixes_" + querytime + string(".xz");
            int sysret = system(cmd.c_str());
            if (sysret == -1)
                cout << "File decompression or download failed" << endl;
        }
    } else if (type.substr(0, 7) == "country") {
        filename_str = type + "_" + querytime;
        it = find(file_names.begin(), file_names.end(), filename_str);
        if (it == file_names.end()) {
            cout << "Start downloading the country resource list" << endl;
            filename_str= DOWNLOAD + string("/") + filename_str + string(".json");
            filename = filename_str.c_str();
            url = "https://stat.ripe.net/data/country-resource-list/data.json?resource=" + type.substr(type.size()-2);
            download_raw_data(filename, url);
            cout << "Finish downloading the country resource list" << endl;
        }
    } else if (type.substr(0, 2) == "as") {
        filename_str = type + "_" + querytime;
        it = find(file_names.begin(), file_names.end(), filename_str);
        if (it == file_names.end()) {
            cout << "Start downloading the as announced prefixes" << endl;
            filename_str= DOWNLOAD + string("/") + filename_str + string(".json");
            filename = filename_str.c_str();
            std::size_t idx = type.find("_");
            url = "https://stat.ripe.net/data/announced-prefixes/data.json?resource=AS" + type.substr(idx+1);
            download_raw_data(filename, url);
            cout << "Finish downloading the as announced prefixes" << endl;
        }
    }
}

string query_file(string query_name, string dir)
{
    vector<string> file_names = get_file_names(dir);
    int time = 0, last_time = 0;
    for (auto i = 0; i < file_names.size(); ++i)
    {
        string::size_type pos_start = file_names[i].find(query_name);
        if (pos_start != string::npos)
        {
            string temp = file_names[i];
            time = stoi(temp.substr(temp.find_last_of("_") + 1));
            last_time = (time > last_time) ? time : last_time;
        }
    }
    if (last_time == 0)
        fatal("There is no relevant data locally, please download first!");
    return to_string(last_time);
}

void query_file_all(string query_name, string dir, vector<string>& file_names) {
    vector<string> temp_file_names = get_file_names(dir);
    for (auto i = 0; i < temp_file_names.size(); ++i) {
        string::size_type pos_start = temp_file_names[i].find(query_name);
        if (pos_start != string::npos) {
            file_names.push_back(temp_file_names[i]);
        }
    }
    if (file_names.size() == 0)
        fatal("There is no relevant data locally, please download first!");
}

string get_hitlist() {
    string file_hitlist;
    file_hitlist = DOWNLOAD + string("/hitlist_") + query_file(string("hitlist_"), DOWNLOAD);
    return file_hitlist;
}

string get_seedset(string type, char* region) {
    string file_seedset;
    if (NULL == region) 
        file_seedset = OUTPUT + string("/seeds_") + type + string("_") + query_file(string("seeds_") + type, OUTPUT);
    else
        file_seedset = OUTPUT + string("/hitlist_") + string(region) + string("_") + type + string("_") + query_file(string("hitlist_") + string(region) + string("_") + type, OUTPUT);
    return file_seedset;
}

string get_seedset(string type) {
    string file_seedset;
    file_seedset = OUTPUT + string("/seeds_") + type + string("_") + query_file(string("seeds_") + type, OUTPUT);
    return file_seedset;
}

string get_aliasfile() {
    string file_alias;
    file_alias = DOWNLOAD + string("/aliased_prefixes_") + query_file(string("aliased_prefixes_"), DOWNLOAD);
    return file_alias;
}

string get_countryfile(string scope) {
    string file_country;
    file_country = DOWNLOAD + string("/") + scope + string("_")  + query_file(scope, DOWNLOAD) + string(".json");
    return file_country;
}

void get_countryfile_all(vector<string>& countries) {    
    query_file_all("country", DOWNLOAD, countries);
    for (auto& country : countries) {
        country = DOWNLOAD + string("/") + country;
    }
}

void get_aliasfile_all(vector<string>& aliases) {    
    query_file_all("alias", OUTPUT, aliases);
    for (auto& alias : aliases) {
        alias = OUTPUT + string("/") + alias;
    }
}

string get_asfile(string scope) {
    string file_as;
    file_as = DOWNLOAD + string("/") + scope + string("_")  + query_file(scope, DOWNLOAD) + string(".json");
    return file_as;
}