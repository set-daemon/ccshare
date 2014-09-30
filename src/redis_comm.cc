#include "debug.h"
#include "date_time.h"
#include "redis_comm.h"

int RedisComm::init(const string &ip_addr, int db) {
	if (!ip_addr.empty() && db >= 0) {
		addr.addr.parse(ip_addr);
		addr.db = db;
	}

	reconnect();

	return 0;
}

int RedisComm::choose_db(int db) {
	redisReply *reply = (redisReply*)redisCommand(redis_ctx, "select %d", db);
	//redisReply *reply = (redisReply*)redisCommand(redis_ctx, "select 0");
	if (reply) print_reply(reply);

	if (reply && reply->type == REDIS_REPLY_ERROR) {
		ERROR("[%s] it can not select db[%d] [%s]\n", DateTime().date_str, db, redis_ctx->errstr);
		return -1;
	}

	return 0;
}


int RedisComm::reconnect() {
	int retry_times = 0;
	do {
		redis_ctx = redisConnect(addr.addr.get_ip_c(), addr.addr.get_port());	
		if (redis_ctx && !redis_ctx->err) {
			if (choose_db(addr.db) != 0) {
				close_db();
				return -1;
			}
			return 0;
		}
		ERROR("[%s] it can not connect redis server[%s:%d] as [%s]!\n",
				DateTime().date_str, addr.addr.get_ip_c(), addr.addr.get_port(),
				redis_ctx->errstr);
		close_db();
	} while (++retry_times < max_reconnect_times);

	return -1;
}

int RedisComm::exec(char *cmd_line, RedisCmd *cmd, void *dest) {
	if (!redis_ctx) {
		if (reconnect() != 0) {
			return -1;
		}
	}

	INFO("[%s] it will execute command [%s]!\n", DateTime().date_str, cmd_line);
	redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd_line);
	if (reply) print_reply(reply);
	if (NULL == reply || reply->type == REDIS_REPLY_ERROR) {
		ERROR("[%s] first, execute command [%s] failed!\n", DateTime().date_str, cmd_line);
		if (reconnect() != 0) {
			return -1;
		}
		if (reply) freeReplyObject(reply);
		reply = (redisReply*)redisCommand(redis_ctx, cmd_line);
		if (NULL == reply || reply->type == REDIS_REPLY_ERROR) {
			if (reply) freeReplyObject(reply);
			ERROR("[%s] redis execute [%s] failed![%s]\n", DateTime().date_str,
					cmd_line, redis_ctx->errstr);
			return -1;
		}
	}

	int ret = 0;
	if (cmd) {
		ret = cmd->reply_parser(cmd, reply, dest);
	}
	freeReplyObject(reply);

	return ret;

	return 0;
}

#if 0
int RedisComm::exec(const string &command,
					reply_parser_func parser,
					void *dest) {
	if (!redis_ctx) {
		if (reconnect() != 0) {
			return -1;
		}
	}

	redisReply* reply = (redisReply*)redisCommand(redis_ctx, command.c_str());
	if (reply) print_reply(reply);
	if (NULL == reply || reply->type == REDIS_REPLY_ERROR) {
		ERROR("[%s] first, execute command [%s] failed!\n", DateTime().date_str, command.c_str());
		if (reconnect() != 0) {
			return -1;
		}
		if (reply) freeReplyObject(reply);
		reply = (redisReply*)redisCommand(redis_ctx, command.c_str());
		if (NULL == reply || reply->type == REDIS_REPLY_ERROR) {
			if (reply) freeReplyObject(reply);
			ERROR("[%s] redis execute [%s] failed![%s]\n", DateTime().date_str,
					command.c_str(), redis_ctx->errstr);
			return -1;
		}
	}

	int ret = 0;
	if (parser) {
		ret = parser(*reply, dest);
	}
	freeReplyObject(reply);

	return ret;
}
#endif
