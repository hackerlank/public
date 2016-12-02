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

#if !(defined(_WIN32)||defined(_WIN64))
typedef long long           PORT_LONGLONG;
#endif
#include <hiredis/hiredis.h>
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

#define DO_RET_REF_ARG1(CMD,RETURN,arg0,arg1) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key,arg0,&arg1); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
		return RETURN; \
	}

#define DO_RET_ARG1(CMD,RETURN,arg0,arg1) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key,arg0,arg1); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
		return RETURN; \
	}

#define DO_RET_ARG0(CMD,RETURN,arg0) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key,arg0); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
		return RETURN; \
	}

#define DO_RET_REF_ARG0(CMD,RETURN,arg0) \
	if(!redis){ \
		KEYE_LOG("redis not connected\n"); \
		return RETURN; \
	} \
	try{ \
		return redis->CMD(key,&arg0); \
	} catch(r3c::CRedisException& ex){ \
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
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
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
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
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
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
		KEYE_LOG("redis_proxy::"#CMD" ERROR: %s\n",ex.str().c_str()); \
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
        
        //mute debug log
        r3c::set_debug_log_write(nullptr);
        
        //test connection
        std::pair<std::string, uint16_t> node;
        unsigned int slot = 0;//redis->cluster_mode()? redis->get_key_slot(key): 0;
        redisContext* redis_context = redis->get_redis_context(slot, &node);
        if (NULL == redis_context||redis_context->err != 0)
            throw r3c::CRedisException(r3c::ERROR_CONNECT_REDIS, "refused", __FILE__, __LINE__);

		return true;
	}catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::connect ERROR: %s\n",ex.str().c_str());
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
		KEYE_LOG("redis_proxy::command ERROR: %s\n",ex.str().c_str());
		return -1;
	}
}

//distributed lock
bool redis_proxy::lock(const char* key,int seconds){
    char lkey[128];
    sprintf(lkey,"%s:lock",key);
    if(setnx(lkey,"1")){
        //locked, set expire
        expire(lkey,seconds);
        return true;
    }else{
        //failed, check
        char cmd[64];
        sprintf(cmd,"ttl %s",lkey);
        auto redis_res=redis->redis_command(REDIS_REPLY_INTEGER,nullptr,nullptr,nullptr,cmd);
        if(redis_res && redis_res->integer<=0)
            //crashed by some reason, revive expire
            expire(lkey,seconds);
        return false;
    }
}

void redis_proxy::unlock(const char* key){
    //must pair with lock
    char lkey[128];
    sprintf(lkey,"%s:lock",key);
    del(lkey);
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

bool redis_proxy::setnx(const char* key,const char* value){
	DO_RET_ARG0(setnx,false,value)
}	//set if not exists

int64_t redis_proxy::incrby(const char* key,int64_t value){
    DO_RET_ARG0(incrby,0,value)
}

bool redis_proxy::get(const char* key,std::string& value){
	DO_RET_REF_ARG0(get,false,value)
}

// list
int redis_proxy::llen(const char* key){
	DO_RET(llen,-1)
}

bool redis_proxy::lpop(const char* key,std::string& value){
	DO_RET_REF_ARG0(lpop,false,value)
}		//pop from front

int redis_proxy::lpush(const char* key,const std::vector<std::string>& values){
	DO_RET_ARG0(lpush,-1,values)
}

int redis_proxy::lpushx(const char* key,const std::vector<std::string>& values){
	//DO_RET_ARG0(lpushx,-1,values)
	//not implemented yet
	return -1;
}				//push if list exists

int redis_proxy::lrange(const char* key,int start,int end,std::vector<std::string>& values){
	CHECK_INT
	try{
		return redis->lrange(key,start,end,&values);
	} catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::lrange ERROR: %s\n",ex.str().c_str());
		return -1;
	}
}	//get range elements

bool redis_proxy::ltrim(const char* key,int start,int end){
	CHECK_BOOL
	try{
		return redis->ltrim(key,start,end);
	} catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::ltrim ERROR: %s\n",ex.str().c_str());
		return -1;
	}
}		//remove range elements

bool redis_proxy::rpop(const char* key,std::string& value){
	DO_RET_REF_ARG0(rpop,false,value)
}		//pop from back

int redis_proxy::rpush(const char* key,const char* value){
	DO_RET_ARG0(rpush,-1,value)
}

int redis_proxy::rpush(const char* key,const std::vector<std::string>& values){
	DO_RET_ARG0(rpush,-1,values)
}

int redis_proxy::rpushx(const char* key,const char* value){
	DO_RET_ARG0(rpushx,-1,value)
}

// hash
int redis_proxy::hlen(const char* key){
	DO_RET(hlen,-1)
}

int redis_proxy::hdel(const char* key,const std::vector<std::string>& fields){
	DO_RET_ARG0(hdel,-1,fields)
}

bool redis_proxy::hexists(const char* key,const char* field){
	DO_RET_ARG0(hexists,false,field)
}						//if exists field

bool redis_proxy::hset(const char* key,const char* field,const char* value){
	DO_RET_ARG1(hset,false,field,value)
}

bool redis_proxy::hsetnx(const char* key,const char* field,const char* value){
	DO_RET_ARG1(hsetnx,false,field,value)
}

bool redis_proxy::hget(const char* key,const char* field,std::string& value){
	DO_RET_ARG1(hget,false,field,&value)
}

void redis_proxy::hmset(const char* key,const std::map<std::string,std::string>& map){
	DO_ARG0(hmset,map)
}	//set multiple values

int redis_proxy::hmget(const char* key,const std::vector<std::string>& fields,std::map<std::string,std::string>& map){
	DO_RET_ARG1(hmget,false,fields,&map)
}

int64_t redis_proxy::hincrby(const char* key,const char* field,int64_t value){
    DO_RET_ARG1(hincrby,0,field,value)
}

int redis_proxy::hgetall(const char* key,std::map<std::string,std::string>& map){
	DO_RET_REF_ARG0(hgetall,-1,map)
}		//get all

int redis_proxy::hkeys(const char* key,std::vector<std::string>& fields){
	DO_RET_REF_ARG0(hkeys,-1,fields)
}				//get all keys

int redis_proxy::hvals(const char* key,std::vector<std::string>& vals){
	DO_RET_REF_ARG0(hvals,-1,vals)
}				//get all values

// set
int redis_proxy::sadd(const char* key,const std::vector<std::string>& values){
	DO_RET_ARG0(sadd,-1,values)
}

int redis_proxy::scard(const char* key){
	DO_RET(scard,-1)
}	//length

bool redis_proxy::sismember(const char* key,const std::string& value){
	DO_RET_ARG0(sismember,false,value)
}

int redis_proxy::smembers(const char* key,std::vector<std::string>& values){
	DO_RET_REF_ARG0(smembers,-1,values)
}

int redis_proxy::spop(const char* key,int count,std::vector<std::string>& values){
	DO_RET_ARG1(spop,-1,count,&values)
}

int redis_proxy::srem(const char* key,const std::vector<std::string>& values){
	DO_RET_ARG0(srem,-1,values)
}		//remove

// sort set
int redis_proxy::zadd(const char* key,const std::map<std::string,int64_t>& map){
	DO_RET_ARG0(zadd,-1,map)
}

int redis_proxy::zcard(const char* key){
	//DO_RET(zcard,-1)
	//not implemented yet
	return -1;
}	//length

int redis_proxy::zcount(const char* key,int64_t min,int64_t max){
	DO_RET_ARG1(zcount,-1,min,max)
}	//count range

int redis_proxy::zrange(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec){
	CHECK_INT;
	try{
		return redis->zrange(key,start,end,withscores,&vec);
	} catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::zrange ERROR: %s\n",ex.str().c_str());
		return -1;
	}
}

int redis_proxy::zrevrange(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec){
	CHECK_INT;
	try{
		return redis->zrevrange(key,start,end,withscores,&vec);
	} catch(r3c::CRedisException& ex){
		KEYE_LOG("redis_proxy::zrevrange ERROR: %s\n",ex.str().c_str());
		return -1;
	}
}

int redis_proxy::zrank(const char* key,const char* field){
	DO_RET_ARG0(zrank,-1,field)
}		//get rank

int redis_proxy::zrevrank(const char* key,const char* field){
	DO_RET_ARG0(zrevrank,-1,field)
}		//get reverse rank

long long redis_proxy::zscore(const char* key,const char* field){
	DO_RET_ARG0(zscore,-1,field)
}		//get value
