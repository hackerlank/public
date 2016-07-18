// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: redis_proxy.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#ifndef _redis_proxy_h_
#define _redis_proxy_h_

namespace r3c{
	class CRedisClient;
};

template class KEYE_API std::shared_ptr<r3c::CRedisClient>;
namespace keye{
// --------------------------------------------------------
// redis_proxy:multi-thread async mysql proxy
// --------------------------------------------------------
class KEYE_API redis_proxy: public vic_proxy{
public:
				redis_proxy(unsigned char threads=1);
	//open multi-connections to database,host=[ip:port],don't need port
	bool		connect(const char* host,unsigned short port=0,const char* user=nullptr,const char* passwd=nullptr,const char* dbname=nullptr);
	// raw command
	int			command(result_t&,const char* cmd);

	//key
	bool	exists		(const char* key);
	bool	expire		(const char* key,uint32_t seconds);
	bool	del			(const char* key);

	//string
	void	set			(const char* key,const char* value);
	bool	setnx		(const char* key,const char* value);	//set if not exists
	bool	get			(const char* key,std::string& value);

	// list
	int		llen		(const char* key);
	bool	lpop		(const char* key,std::string& value);		//pop from front
	int		lpush		(const char* key,const std::vector<std::string>& values);
	int		lpushx		(const char* key,const std::vector<std::string>& values);				//push if list exists
	int		lrange		(const char* key,int start,int end,std::vector<std::string>& values);	//get range elements
	bool	ltrim		(const char* key,int start,int end);		//remove range elements
	bool	rpop		(const char* key,std::string& value);		//pop from back
	int		rpush		(const char* key,const char* value);
	int		rpush		(const char* key,const std::vector<std::string>& values);
	int		rpushx		(const char* key,const char* value);

	// hash
	int		hlen		(const char* key);
	int		hdel		(const char* key,const std::vector<std::string>& fields);
	bool	hexists		(const char* key,const char* field);						//if exists field
	bool	hset		(const char* key,const char* field,const char* value);
	bool	hsetnx		(const char* key,const char* field,const char* value);
	bool	hget		(const char* key,const char* field,std::string& value);
	void	hmset		(const char* key,const std::map<std::string,std::string>& map);	//set multiple values
	int		hmget		(const char* key,const std::vector<std::string>& fields,std::map<std::string,std::string>& map);
	int		hgetall		(const char* key,std::map<std::string,std::string>& map);		//get all
	int		hkeys		(const char* key,std::vector<std::string>& fields);				//get all keys
	int		hvals		(const char* key,std::vector<std::string>& vals);				//get all values

	// set
	int		sadd		(const char* key,const std::vector<std::string>& values);
	int		scard		(const char* key);	//length
	bool	sismember	(const char* key,const std::string& value);
	int		smembers	(const char* key,std::vector<std::string>& values);
	int		spop		(const char* key,int count,std::vector<std::string>& values);
	int		srem		(const char* key,const std::vector<std::string>& values);		//remove

	// sort set
	int		zadd		(const char* key,const std::map<std::string,int64_t>& map);
	int		zcard		(const char* key);	//length
	int		zcount		(const char* key,int64_t min,int64_t max);	//count range
	int		zrange		(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec);
	int		zrevrange	(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec);
	int		zrank		(const char* key,const char* field);		//get rank
	int		zrevrank	(const char* key,const char* field);		//get reverse rank
	long long zscore	(const char* key,const char* field);		//get value
protected:
	std::shared_ptr<r3c::CRedisClient>	redis;
};
// --------------------------------------------------------
};// namespace
#endif // _redis_proxy_h_
