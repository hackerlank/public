// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: redis_proxy.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#include "stdafx.h"

#include <hiredis/hiredis.h>
//#include <hiredis/Win32_Interop/win32fixes.h>
//#include "win32fixes.hpp"
#include "r3c.h"

#include <libvic/libvic_fwd.h>

using namespace keye;

void copy(result_t& dest,const redisReply& src){

}

#define CHECK(RETURN) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	}
#define CHECK_VOID	CHECK()
#define CHECK_BOOL	CHECK(false)
#define CHECK_INT	CHECK(-1)

#define DO_RET_ARG0(CMD,RETURN,arg0) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key,arg0); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s",ex.str().c_str()); \
		return RETURN; \
	}

#define DO_RET(CMD,RETURN) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s",ex.str().c_str()); \
		return RETURN; \
	}

#define DO_ARG0(CMD,arg0) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return; \
	} \
	try{ \
		return redis->CMD(key,arg0); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s",ex.str().c_str()); \
		return; \
	}

#define DO(CMD) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return; \
	} \
	try{ \
		redis->CMD(key); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s",ex.str().c_str()); \
	}

// --------------------------------------------------------
// redis_proxy
// --------------------------------------------------------
redis_proxy::redis_proxy(unsigned char conns){
}

bool redis_proxy::connect(const char* host, unsigned short port,
		const char* user, const char* passwd,const char* db){
	try{
		redis.reset(new r3c::CRedisClient(host));
		return true;
	}catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::connect ERROR: %s",ex.str().c_str());
		return false;
	}
}

int redis_proxy::command(result_t& result,const char* cmd){
	CHECK_INT
	try{
		auto redis_res=redis->redis_command(REDIS_REPLY_STRING,nullptr,nullptr,nullptr,cmd);
		if(redis_res)copy(result,*redis_res);
		return redis_res!=nullptr;
	} catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::command ERROR: %s",ex.str().c_str());
		return -1;
	}
}

//key
bool redis_proxy::exists(const char* key){
	DO_RET(exists,false)
}

bool redis_proxy::expire(const char* key,uint32_t seconds){
	DO_RET_ARG0(expire,false,seconds)
}

bool redis_proxy::del(const char* key){
	DO_RET(del,false)
}

//string							
void redis_proxy::set(const char* key,const char* value){
	DO_ARG0(set,value)
}

bool redis_proxy::setnx(const char* key,const char* value){return 0;}	//set if not exists
bool redis_proxy::get(const char* key,std::string& value){return 0;}

// list
int redis_proxy::llen(const char* key){return 0;}
bool redis_proxy::lpop(const char* key,std::string& value){return 0;}		//pop from front
int redis_proxy::lpush(const char* key,const std::vector<std::string>& values){return 0;}
int redis_proxy::lpushx(const char* key,const std::vector<std::string>& values){return 0;}				//push if list exists
int redis_proxy::lrange(const char* key,int start,int end,std::vector<std::string>& values){return 0;}	//get range elements
bool redis_proxy::ltrim(const char* key,int start,int end){return 0;}		//remove range elements
bool redis_proxy::rpop(const char* key,std::string& value){return 0;}		//pop from back
int redis_proxy::rpush(const char* key,const char* value){return 0;}
int redis_proxy::rpush(const char* key,const std::vector<std::string>& values){return 0;}
int redis_proxy::rpushx(const char* key,const char* value){return 0;}

// hash
int redis_proxy::hlen(const char* key){return 0;}
int redis_proxy::hdel(const char* key,const std::vector<std::string>& fields){return 0;}
bool redis_proxy::hexists(const char* key,const char* field){return 0;}						//if exists field
bool redis_proxy::hset(const char* key,const char* field,const char* value){return 0;}
bool redis_proxy::hsetnx(const char* key,const char* field,const char* value){return 0;}
bool redis_proxy::hget(const char* key,const char* field,std::string& value){return 0;}
void redis_proxy::hmset(const char* key,const std::map<std::string,std::string>& map){}	//set multiple values
int redis_proxy::hmget(const char* key,const std::vector<std::string>& fields,std::map<std::string,std::string>& map){return 0;}
int redis_proxy::hgetall(const char* key,std::map<std::string,std::string>& map){return 0;}		//get all
int redis_proxy::hkeys(const char* key,std::vector<std::string>& fields){return 0;}				//get all keys
int redis_proxy::hvals(const char* key,std::vector<std::string>& vals){return 0;}				//get all values

// set
int redis_proxy::sadd(const char* key,const std::vector<std::string>& values){return 0;}
int redis_proxy::scard(const char* key){return 0;}	//length
bool redis_proxy::sismember(const char* key,const std::string& value){return 0;}
int redis_proxy::smembers(const char* key,std::vector<std::string>& values){return 0;}
int redis_proxy::spop(const char* key,int count,std::vector<std::string>& values){return 0;}
int redis_proxy::srem(const char* key,const std::vector<std::string>& values){return 0;}		//remove

// sort set
int redis_proxy::zadd(const char* key,const std::map<std::string,int64_t>& map){return 0;}
int redis_proxy::zcard(const char* key){return 0;}	//length
int redis_proxy::zcount(const char* key,int64_t min,int64_t max){return 0;}	//count range
int redis_proxy::zrange(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec){return 0;}
int redis_proxy::zrevrange(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec){return 0;}
int redis_proxy::zrank(const char* key,const char* field){return 0;}		//get rank
int redis_proxy::zrevrank(const char* key,const char* field){return 0;}		//get reverse rank
long long redis_proxy::zscore(const char* key,const char* field){return 0;}		//get value
