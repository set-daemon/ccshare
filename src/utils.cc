#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common_defs.h"
#include "debug.h"
#include "utils.h"

namespace Utils {

int signal_process() {
	// TODO:

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    umask(0);
	return E_OK;
}

int set_daemon(bool set_flag) {
	// TODO:
	if (!set_flag) {
        return 0;
    }

    pid_t pid;

    switch (pid=fork()) {
        case 0:
            break;
        case -1:
            exit(-1);
        default:
            exit(0); // parent process.
    }

    int i = 0;
    for (i = 0; i < NOFILE; ++i) {
        close(i);
    }

    setsid();

	signal_process();

	return E_OK;
}

int check_path_is_valid(const char* path) {
	// TODO:

	return E_OK;
}

int change_working_dir(const char *working_path) {
	// TODO:

	return E_OK;
}

bool parse_line(const string &line, const char* spliter, vector<string> &storage) {
	size_t pos = 0;

	int quit = 0;
	while (!quit) {
		size_t next_pos = line.find_first_of(*spliter, pos);
		if (next_pos == string::npos) {
			next_pos = line.size();
			quit = 1;
		}
		size_t space_pos = line.find_first_not_of(" ", pos);
		size_t last_space_pos = line.find_last_not_of(" ", next_pos-1);
		string sec = line.substr(space_pos, last_space_pos-space_pos+1);
		//INFO(",,,, size=%d, sec = {%s}, pos=%d,next_pos=%d\n", line.size(), sec.c_str(), pos, next_pos);
		storage.push_back(sec);
		pos = next_pos + 1;
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, set<string> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.insert(vec[i]);
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, vector<int> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.push_back(strtoul(vec[i].c_str(), NULL, 10));
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, set<int> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.insert(strtoul(vec[i].c_str(), NULL, 10));
	}


	return true;
}

bool parse_line(const string &line, const char* spliter, map<string, string> &kvs) {
	vector<string> segs;

	if (true != parse_line(line, spliter, segs)) {
		return false;
	}

	for (int i = 0; i < segs.size(); i++) {
		vector<string> kv;
		if (true != parse_line(segs[i], "=", kv)) {
			return false;
		}

		if (kv.size() < 2) {
			kvs[kv[0]] = "";
		} else {
			kvs[kv[0]] = kv[1];
		}
	}

	return true;
}

string to_string(int v) {
	char buf[24] = { 0 };
	sprintf(buf, "%d", v);

	return string(buf);
}

string to_string(long v) {
	char buf[24] = { 0 };
	sprintf(buf, "%ld", v);

	return string(buf);
}

string to_string(long long v) {
	char buf[24] = { 0 };
	sprintf(buf, "%lld", v);

	return string(buf);
}

string to_string(float v) {
	char buf[24] = { 0 };
	sprintf(buf, "%f", v);

	return string(buf);
}

string to_string(double v) {
	char buf[24] = { 0 };
	sprintf(buf, "%lf", v);

	return string(buf);
}

string to_string(long double v) {
	char buf[24] = { 0 };
	sprintf(buf, "%llf", v);

	return string(buf);
}

string to_upper(string &str) {
	for (int i = 0; i < str.size(); i++) {
		str[i] = toupper(str[i]);
	}

	return str;
}

string str_ltrim(const string &str, const string &trim_chars) {
	size_t pos = str.find_first_not_of(trim_chars);
	if (pos == string::npos) {
		return str;
	} else {
		return str.substr(pos);
	}
}

string str_rtrim(const string &str, const string &trim_chars) {
	size_t pos = str.find_last_not_of(trim_chars, str.size()-1);
	if (pos == string::npos) {
		return str;
	} else {
		return str.substr(0, pos+1);
	}
}

string str_trim(const string &str, const string &trim_chars) {
	string new_str = str_ltrim(str, trim_chars);
	new_str = str_rtrim(new_str, trim_chars);

	return new_str;
}

}
