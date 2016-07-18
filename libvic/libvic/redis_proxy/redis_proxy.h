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

namespace keye{
// --------------------------------------------------------
// redis_proxy:multi-thread async mysql proxy
// --------------------------------------------------------
namespace r3c{
	class CRedisClient;
};

class KEYE_API redis_proxy: public vic_proxy{
public:
				redis_proxy(unsigned char threads=1);
	//open multi-connections to database
	bool		connect(const char* host,unsigned short port,const char* user,const char* passwd,const char* dbname=nullptr);
	// raw command
	int			command(result_t&,const char* cmd);

	//key
	bool	exists		(const char* key,const char* table=nullptr);
	bool	expire		(const char* key,uint32_t seconds,const char* table=nullptr);
	bool	del			(const char* key,const char* table=nullptr);

	//string
	void	set			(const char* key,const char* value,const char* table=nullptr);
	bool	setnx		(const char* key,const char* value,const char* table=nullptr);		//set if not exists
	bool	get			(const char* key,std::string& value,const char* table=nullptr);

	// list
	int		llen		(const char* key,const char* table=nullptr);
	bool	lpop		(const char* key,std::string& value,const char* table=nullptr);		//pop from front
	int		lpush		(const char* key,const std::vector<std::string>& values,const char* table=nullptr);
	int		lpushx		(const char* key,const std::vector<std::string>& values,const char* table=nullptr);				//push if list exists
	int		lrange		(const char* key,int start,int end,std::vector<std::string>& values,const char* table=nullptr);	//get range elements
	bool	ltrim		(const char* key,int start,int end,const char* table=nullptr);		//remove range elements
	bool	rpop		(const char* key,std::string& value,const char* table=nullptr);		//pop from back
	int		rpush		(const char* key,const char* value,const char* table=nullptr);
	int		rpush		(const char* key,const std::vector<std::string>& values,const char* table=nullptr);
	int		rpushx		(const char* key,const char* value,const char* table=nullptr);

	// hash
	int		hlen		(const char* key,const char* table=nullptr);
	int		hdel		(const char* key,const std::vector<std::string>& fields,const char* table=nullptr);
	bool	hexists		(const char* key,const char* field,const char* table=nullptr);								//if exists field
	bool	hset		(const char* key,const char* field,const char* value,const char* table=nullptr);
	bool	hsetnx		(const char* key,const char* field,const char* value,const char* table=nullptr);
	bool	hget		(const char* key,const char* field,std::string& value,const char* table=nullptr);
	void	hmset		(const char* key,const std::map<std::string,std::string>& map,const char* table=nullptr);	//set multiple values
	int		hmget		(const char* key,const std::vector<std::string>& fields,std::map<std::string,std::string>& map,const char* table=nullptr);
	int		hgetall		(const char* key,std::map<std::string,std::string>& map,const char* table=nullptr);			//get all
	int		hkeys		(const char* key,std::vector<std::string>& fields,const char* table=nullptr);				//get all keys
	int		hvals		(const char* key,std::vector<std::string>& vals,const char* table=nullptr);					//get all values

	// set
	int		sadd		(const char* key,const std::vector<std::string>& values,const char* table=nullptr);
	int		scard		(const char* key,const char* table=nullptr);	//length
	bool	sismember	(const char* key,const std::string& value,const char* table=nullptr);
	int		smembers	(const char* key,std::vector<std::string>& values,const char* table=nullptr);
	int		spop		(const char* key,int count,std::vector<std::string>& values,const char* table=nullptr);
	int		srem		(const char* key,const std::vector<std::string>& values,const char* table=nullptr);			//remove

	// sort set
	int		zadd		(const char* key,const std::map<std::string,int64_t>& map,const char* table=nullptr);
	int		zcard		(const char* key,const char* table=nullptr);	//length
	int		zcount		(const char* key,int64_t min,int64_t max,const char* table=nullptr);	//count range
	int		zrange		(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec,const char* table=nullptr);
	int		zrevrange	(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec,const char* table=nullptr);
	int		zrank		(const char* key,const char* field,const char* table=nullptr);			//get rank
	int		zrevrank	(const char* key,const char* field,const char* table=nullptr);			//get reverse rank
	long long zscore	(const char* key,const char* field,const char* table=nullptr);			//get value
protected:
	std::shared_ptr<r3c::CRedisClient>	redis;
};
// --------------------------------------------------------
};// namespace
#endif // _redis_proxy_h_
