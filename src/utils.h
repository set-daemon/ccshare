/*
 * file name: utils.h
 * purpose  : 
 * author   : huangchunping
 * date     : 2014-08-20
 * history  :
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

namespace Utils {

int signal_process();

int set_daemon();

int check_path_is_valid(const char *path);

int change_working_dir(const char *working_path);

bool parse_line(const string &line, const char* spliter, vector<string> &storage);
bool parse_line(const string &line, const char* spliter, set<string> &storage);
bool parse_line(const string &line, const char* spliter, vector<int> &storage);
bool parse_line(const string &line, const char* spliter, set<int> &storage);
// it works for such line: k=1;m=3
bool parse_line(const string &line, const char* spliter, map<string, string> &kvs);

string str_ltrim(const string &str, const string &trim_chars);
string str_rtrim(const string &str, const string &trim_chars);
string str_trim(const string &str, const string &trim_chars);

string to_string(int v);
string to_string(long v);
string to_string(long long v);
string to_string(float v);
string to_string(double v);
string to_string(long double v);
string to_upper(string &str);

}

#endif // __UTILS_H__
