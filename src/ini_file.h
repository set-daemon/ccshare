/*
 * file name: ini_file.h
 * purpose  : 
 * author   : huang chunping
 * date     : 2014-08-28
 * history  :
 */

#ifndef __INI_FILE_H__
#define __INI_FILE_H__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <map>
#include <fstream>
#include <sstream>
#include <string>

#include "debug.h"
#include "utils.h"

class IniFile {
public:
	typedef std::map<std::string, std::string> KeyType;
	typedef std::map<std::string, KeyType> SectionType;

public:
	IniFile() {}
	~IniFile() {}

	inline bool init(const std::string &file);

	std::string get(const std::string &section, const std::string &key, std::string value) {
		SectionType::iterator sit = data_.find(section);
		if (sit != data_.end()) {
			KeyType::iterator kit = sit->second.find(key);
		if (kit != sit->second.end())
			value = kit->second;
		}
		return value;
	}

	int get(const std::string &section, const std::string &key, int value) {
		std::string str_value = get(section, key, "");
		if (str_value != "") {
			value = strtoul(str_value.c_str(), NULL, 10);
		}
		
		return value;
	}

	double get(const std::string &section, const std::string &key, double value) {
		std::string str_value = get(section, key, "");
		if (str_value != "") {
			value = strtod(str_value.c_str(), NULL);
		}

		return value;
	}

private:
	SectionType data_; // < section, <key, value> >
};

bool IniFile::init(const std::string &file)
{
	std::ifstream in(file.c_str());
	if (!in) {
		return false;
	}
	char c;
	std::string line, section, key, value;

	while (getline(in, line)) {
#if 1
		if (line.empty() || line[0] == '\n' || line[0] == '#' || line[0] == ';') {
			//INFO(",,,comment or space line!\n");
			continue;	
		}
		line = Utils::str_trim(line, " \t\n");
		if (line.empty() || line[0] == '\n' || line[0] == '#' || line[0] == ';') {
			//INFO(",,,comment or space line!\n");
			continue;
		}
		//INFO(",,,line = %s\n", line.c_str());

		if (line[0] == '[' && line[line.size()-1] == ']') {
			section = Utils::str_trim(line.substr(1, line.size()-2), " \t");
			//INFO(",,,section = %s\n", section.c_str());
			continue;
		} else if (line[0] == '[') {
			return false;
		}

		size_t pos = line.find_first_of('=', 0);
		if (pos == string::npos) {
			ERROR("it can not find equalifier for line [%s]\n", line.c_str());
			return false;
		}
		if (section.empty()) {
			return false;
		}
		string key = Utils::str_trim(line.substr(0, pos), "\t ");
		string val = Utils::str_trim(line.substr(pos+1, line.size() - pos), "\t ");
		data_[section][key] = val;
		//INFO("[%s] section[%s], segment[k={%s},val={%s}]\n",
		//	  DateTime().date_str, section.c_str(), key.c_str(), val.c_str());
#else
		std::istringstream is(line);
		if (is >> c) {
			if (c == '=') {
				return false;
			}
			if (c == '#' || c == ';') { // <commentary>
				continue;
			} else if (c == '[') { // section
				if ((is >> section) && (section[section.size() - 1] == ']')) {
					section.erase(section.size() - 1);
					data_[section];
					continue;
				} else {
					return false;
				}
			} else {
				is.putback(c);
				if (is >> key) {
					std::string::size_type pos = key.find('=');
					if (pos == std::string::npos) { // key = value or key =value
						if (is >> c >> value && c == '=') {
							data_[section][key] = value;
							continue;
						}
					} else if (pos == key.size() - 1) { // key= value
						key.erase(key.size() - 1);
						if (is >> value) {
							data_[section][key] = value;
							continue;
						}
					} else { // key=value
						data_[section][key.substr(0, pos)] = key.substr(pos + 1);
						continue;
					}
					return false;
				}
			}
		}
#endif
	}

	return true;
}

#endif // __INI_FILE_H__
