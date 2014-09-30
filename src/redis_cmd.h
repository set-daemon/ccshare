/*
 * file name: redis_cmd.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2014-09-26
 * history  :
 *            1. it supports hmget/hgetall now.
 */
#ifndef __REDIS_CMD_H__
#define __REDIS_CMD_H__

#include <string>
#include <vector>
using namespace std;

#include <hiredis.h>

#include "ini_file.h"
#include "debug.h"
#include "date_time.h"
#include "data_op.h"

typedef enum {
	HMGET = 0x00,
	HGETALL,
	KEYS,
	UNKNOWN_REDIS_CMD
}RedisCmdId;

class RedisCmd;
typedef int (*reply_parser_func)(RedisCmd* /*cmd*/, redisReply* /*reply*/, void* /*ctx*/);
#define DEF_PARSER_FUNC(func_name) int func_name(RedisCmd* cmd, redisReply* reply, void* ctx)

class RedisCmd {
public:
	RedisCmd(const string &_cmd_name) : cmd_name(_cmd_name),
							reply_parser((reply_parser_func)0){
	}
	RedisCmd() : cmd_name("") {
	}

	~RedisCmd() {
	}

	int format_cmd(char *buf, int buf_size, ...);

	attr_set_func get_set_func(const string &attr_name) {
		map<string, attr_set_func>::iterator iter = arg_op_map.find(attr_name);
		if (iter == arg_op_map.end()) {
			return (attr_set_func)0;
		}

		iter->second;
	}
	
	void print() {
		INFO("[%s] redis-cmd: [%d]%s->[%s]\n",
			 DateTime().date_str, cmd_id, cmd_name.c_str(), cmd_line.c_str());
	}

	// cmd template format: sysfunction redis_op key ....
	// for example: "keys %s", the key pattern can be set by calling format_cmd.
	//              "hmget ad:%08x id gid", it can be completed by calling format_cmd.
	static RedisCmd* cmd_generator(const string& redis_cmd_line);

public:
	RedisCmdId 			        cmd_id;
	string 				        cmd_line;
	string 				        cmd_name;
	string 				        cmd_op;
	string          	        op_key;
	map<string, attr_set_func>  arg_op_map;
	vector<string> 			 	arg_names;
	vector<attr_set_func> 		set_ops;
	reply_parser_func    		reply_parser;
};

class RedisCmdFactory {
public:
	~RedisCmdFactory() {
	}

	static RedisCmdFactory* inst() {
		static RedisCmdFactory factory;

		return &factory;
	}

	int init(const string &conf_file) {
		IniFile processor;

		if (true != processor.init(conf_file)) {
			ERROR("[%s] it can not load config file [%s]\n",
					DateTime().date_str, conf_file.c_str());
			return -1;
		}

		int op_num = processor.get("redis-ops", "op-num", 0);
		char op_seg[64];
		for (int i = 0; i < op_num; i++) {
			int ret = snprintf(op_seg, sizeof(op_seg)-1, "op-%d", i);
			if (ret < 0 || ret >= sizeof(op_seg)-1) {
				return -1;
			}
			string cmd_line = processor.get("redis-ops", op_seg, "");
			if (cmd_line.empty()) {
				continue;
			}
			INFO("seg[%s] cmd_line = %s\n", op_seg, cmd_line.c_str());
			RedisCmd* new_cmd = RedisCmd::cmd_generator(cmd_line);
			if (NULL == new_cmd) {
				ERROR("[%s] it can not generate redis cmd [%s] correctly!\n",
						DateTime().date_str, cmd_line.c_str());
				continue;
			}
			cmds[new_cmd->cmd_name] = new_cmd;
		}

		return 0;
	}

	RedisCmd* get_cmd(const string &cmd_name) {
		map<string, RedisCmd*>::iterator iter = cmds.find(cmd_name);	
		if (iter == cmds.end()) {
			return NULL;
		}
	
		return iter->second;
	}

private:
	RedisCmdFactory() {
	}

	map<string, RedisCmd*> cmds;
};

#endif // __REDIS_CMD_H__
