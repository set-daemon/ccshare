#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <ctype.h>

#include <algorithm>
using namespace std;

#include <hiredis.h>

#include "debug.h"
#include "utils.h"
#include "date_time.h"
#include "redis_cmd.h"
/*
cmd_op parameters  return-type
DEL      1~N		 integer
DUMP     1           nil or string
EXISTS	 1           integer
EXPIRE   2			 integer
EXPIREAT 2           integer
KEYS     1           (string)array
MOVE     2           integer
APPEND   2           integer
BITCOUNT 1           integer
BITOP    3~N         integer
DECR     1           integer
DECRBY   2           integer
GET      1           string
GETBIT   2           integer
GETRANGE 3           string
GETSET   2           string
INCR     1           integer
INCRBY   2           integer
INCRBYFLOAT 2        string
MGET     1~N         (string)array
MSET     2~2N        status(OK)
MSETNX   2~2N        integer
PSETEX   3           status(OK)
SET      2~N         status(OK)
SETBIT   3           integer
SETEX	 3           status(OK)
SETNX    2           integer
SETRANGE 3           integer
STRLEN   1           integer
HDEL     2~N         integer
HEXISTS  2           integer
HGET     2           string
HGETALL  1           (string)array
HINCRBY  3           integer
HINCRBYFLOAT  3      string
HKEYS    1           (string)array
HLEN     1           integer
HMGET    2~N         (string)array
HMSET    3~N         status(OK)
HSET     3         	 integer
HSETNX   3           integer
HVALS    1           (string)array
SELECT   1           status(OK)
*/
static DEF_PARSER_FUNC(hmget_reply_parser)
{
	if (NULL == reply) {
		return -1;
	}

	if (reply->type != REDIS_REPLY_ARRAY) {
		return -1;
	}

	redisReply **elems = reply->element;
	for (int i = 0; i < reply->elements; i++) {
		switch (elems[i]->type) {
			case REDIS_REPLY_NIL:
				cmd->set_ops[i]("", 0, ctx);	
				break;
			case REDIS_REPLY_STRING:
				cmd->set_ops[i](elems[i]->str, elems[i]->len, ctx);	
				break;
			default:
				return -1;
		}
	}

	return 0;
}

static DEF_PARSER_FUNC(hgetall_reply_parser)
{
	if (NULL == reply) {
		return -1;
	}

	if (reply->type != REDIS_REPLY_ARRAY) {
		return -1;
	}
	// the return is k-v mode, so the array should be multiple to 2.
	if (reply->elements % 2 != 0) {
		return -1;
	}
	redisReply **elems = reply->element;
	for (int i = 0; i < reply->elements; i += 2) {
		if (elems[i]->type != REDIS_REPLY_STRING ||
			elems[i+1]->type != REDIS_REPLY_STRING) {
			return -1;
		}

		attr_set_func func = cmd->get_set_func(elems[i]->str);
		if (func) {
			func(elems[i+1]->str, elems[i+1]->len, ctx);
		} else {
			ERROR("[%s] it can not get attr[%s]'s op!\n",
					DateTime().date_str, elems[i]->str);
		}
	}

	return 0;
}

static DEF_PARSER_FUNC(keys_reply_parser)
{
	if (NULL == reply) {
		return -1;
	}

	if (reply->type != REDIS_REPLY_ARRAY) {
		return -1;
	}
	// the return is k-v mode, so the array should be multiple to 2.
	if (reply->elements % 2 != 0) {
		return -1;
	}
	redisReply **elems = reply->element;
	for (int i = 0; i < reply->elements; i++) {
		if (elems[i]->type != REDIS_REPLY_STRING) {
			return -1;
		}
		cmd->set_ops[0](elems[i]->str, elems[i]->len, ctx);
	}

	return 0;
}

struct RedisCmdInfo{
	RedisCmdId 			id;
	char       			*name;
	int        			with_key;
	int                 with_args;
	reply_parser_func 	parser;
} redis_cmds[] = {
	{HMGET, 			"HMGET",   1, 1, hmget_reply_parser},
	{HGETALL, 			"HGETALL", 1, 1, hgetall_reply_parser},
	{KEYS,  			"KEYS",    1, 0, keys_reply_parser}, 
	{UNKNOWN_REDIS_CMD, "",        0, 0, NULL},
};
#define REDIS_CMD_MAP_SIZE sizeof(redis_cmds)/sizeof(redis_cmds[0])

static RedisCmdInfo* get_cmd_info(RedisCmdId id) {
	for (int i = 0; i < REDIS_CMD_MAP_SIZE; i++) {
		if (id == redis_cmds[i].id) {
			return &redis_cmds[i];
		}
	}

	return NULL;
}

static RedisCmdInfo* get_cmd_info(const char *name) {
	for (int i = 0; i < REDIS_CMD_MAP_SIZE; i++) {
		if (strcmp(name, redis_cmds[i].name) == 0) {
			return &redis_cmds[i];
		}
	}

	return NULL;
}

RedisCmd* RedisCmd::cmd_generator(const string& redis_cmd_line) {
	vector<string> str_vec;
	if (true != Utils::parse_line(redis_cmd_line, " ", str_vec)) {
		ERROR("[%s] it can not parse line[%s] succesfully!\n",
				DateTime().date_str, redis_cmd_line.c_str());
		return NULL;
	}

	if (str_vec.size() < 3) {
		ERROR("[%s] there are not enough elements [%d]!\n",
				DateTime().date_str, str_vec.size());
		return NULL;
	}

	string cmd_op = Utils::to_upper(str_vec[1]);
	RedisCmdInfo* cmd_info = get_cmd_info(cmd_op.c_str());
	if (cmd_info->id == UNKNOWN_REDIS_CMD) {
		ERROR("[%s] it can not find redis op[%s]!\n",
				DateTime().date_str, cmd_op.c_str());
		return NULL;
	}

	RedisCmd* cmd = new RedisCmd();
	if (NULL == cmd) {
		return NULL;
	}
	cmd->cmd_id = cmd_info->id;
	cmd->cmd_op = cmd_op;
	cmd->reply_parser = cmd_info->parser;

	int arg_seq = 2;
	cmd->cmd_name = str_vec[0];
	cmd->cmd_line = cmd_op;
	if (cmd_info->with_key) {
		cmd->op_key = str_vec[2];
		cmd->cmd_line += " " + str_vec[2];
		arg_seq = 3;
	}

	for (; arg_seq < str_vec.size(); arg_seq++) {
		vector<string> kv;
		if (true != Utils::parse_line(str_vec[arg_seq], ".", kv)) {
			ERROR("[%s] it can not parse [%s] successfully!\n", DateTime().date_str, str_vec[arg_seq].c_str());
			return NULL;
		}
		if (kv.size() == 1) {
			// we always think struct should exist, so error occurs.
			// TODO: clear previous data created.
			ERROR("[%s] argument[%s] is not formated as struct_name.attr_name!\n", DateTime().date_str, str_vec[arg_seq].c_str());
			return NULL;
		}
		string &struct_name = kv[0];
		string &attr_name = kv[1];

		// attention: like KEYS, it has no arguments.
		if (cmd_info->with_args) {
			cmd->arg_names.push_back(attr_name);
		}

		const DataOpInfo *op_info = DataOpFactory::inst()->get_ops_info(struct_name, attr_name);
		if (NULL == op_info) {
			ERROR("[%s] it can not find op_info for %s.%s!\n",
					DateTime().date_str, struct_name.c_str(), attr_name.c_str());
			return NULL;
		}
		cmd->arg_op_map[attr_name] = op_info->setter;
		cmd->set_ops.push_back(op_info->setter);

		// attention: like KEYS, it has no arguments.
		if (cmd_info->with_args) {
			cmd->cmd_line += " " + attr_name;
		}
		cmd->print();
	}

	return cmd;
}

int RedisCmd::format_cmd(char *buf, int buf_size, ...) {
	va_list va;

	va_start(va, buf_size);
	int ret = vsnprintf(buf, buf_size, cmd_line.c_str(), va);
	if (ret >= buf_size) {
		va_end(va);
		return -1;
	}
	va_end(va);

	return ret;
}
