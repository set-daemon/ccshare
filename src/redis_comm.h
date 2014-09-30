/*
 * file name: redis_comm.h
 * purpose  : 
 * author   : set_daemon@126.com
 * date     : 2014-09-25
 * history  :
 */

#ifndef __REDIS_COMM_H__
#define __REDIS_COMM_H__

#include <string>
using namespace std;

#include <hiredis.h>

#include "debug.h"
#include "date_time.h"
#include "ip_addr.h"
#include "redis_cmd.h"

class RedisAddr {
public:
	RedisAddr() : addr("127.0.0.1:6379"), db(0) {
	}
	RedisAddr(const string &_addr, int _db) :
			addr(_addr), db(_db) {
	}
	~RedisAddr() {
	}

public:
	IpAddr 	addr;
	int		db;
};

class RedisComm {
public:
	RedisComm() : redis_ctx(NULL), addr() {
	}

	RedisComm(const string &ip_addr, int db) : redis_ctx(NULL),
							addr(ip_addr, db), max_reconnect_times(3) {
	}

	~RedisComm() {
	}

	int init(const string &ip_addr, int db);

#if 0
	int exec(const string &command, reply_parser_func parser, void *dest);
#endif

	int exec(char *cmd_line, RedisCmd *cmd, void *dest);

private:
	int reconnect();

	int choose_db(int db);

	int close_db() {
		if (redis_ctx) {
			redisFree(redis_ctx);
			redis_ctx = NULL;
		}
		return 0;
	}

	static void print_reply(redisReply *reply) {
		INFO("[%s] reply str=%s, len=%d, integer=%lld, type=%d, elements=%ld\n",
			  DateTime().date_str, reply->str, reply->len,
			  reply->integer, reply->type, reply->elements);
	}

private:
	redisContext *redis_ctx;
	RedisAddr	 addr;
	int			 max_reconnect_times;
	int			 connect_timeout;
};

#endif // __REDIS_COMM_H__
