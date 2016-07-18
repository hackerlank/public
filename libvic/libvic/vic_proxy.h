// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: vic_proxy.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#ifndef _vic_proxy_h_
#define _vic_proxy_h_

namespace keye{
struct result_t{
	int						type;
	long long				integer;
	std::string				str;
	std::vector<result_t>	elements;
} ;
// --------------------------------------------------------
// db_handler
// --------------------------------------------------------
class KEYE_API db_handler{
public:
	virtual			~db_handler(){}
};
// --------------------------------------------------------
// vic_proxy: database interface
// --------------------------------------------------------
class KEYE_API vic_proxy{
public:
	virtual			~vic_proxy(){}
	//open multi-connections to database
	virtual bool		connect(const char* host,unsigned short port,const char* user,const char* passwd,const char* dbname=nullptr)=0;
	// raw command
	virtual int			command(result_t&,const char* cmd)=0;

	//key
	virtual bool	exists		(const char* key,const char* table=nullptr)=0;
	virtual bool	expire		(const char* key,uint32_t seconds,const char* table=nullptr)=0;
	virtual bool	del			(const char* key,const char* table=nullptr)=0;

	//string
	virtual void	set			(const char* key,const char* value,const char* table=nullptr)=0;
	virtual bool	setnx		(const char* key,const char* value,const char* table=nullptr)=0;		//set if not exists
	virtual bool	get			(const char* key,std::string& value,const char* table=nullptr)=0;

	// list
	virtual int		llen		(const char* key,const char* table=nullptr)=0;
	virtual bool	lpop		(const char* key,std::string& value,const char* table=nullptr)=0;		//pop from front
	virtual int		lpush		(const char* key,const std::vector<std::string>& values,const char* table=nullptr)=0;
	virtual int		lpushx		(const char* key,const std::vector<std::string>& values,const char* table=nullptr)=0;				//push if list exists
	virtual int		lrange		(const char* key,int start,int end,std::vector<std::string>& values,const char* table=nullptr)=0;	//get range elements
	virtual bool	ltrim		(const char* key,int start,int end,const char* table=nullptr)=0;		//remove range elements
	virtual bool	rpop		(const char* key,std::string& value,const char* table=nullptr)=0;		//pop from back
	virtual int		rpush		(const char* key,const char* value,const char* table=nullptr)=0;
	virtual int		rpush		(const char* key,const std::vector<std::string>& values,const char* table=nullptr)=0;
	virtual int		rpushx		(const char* key,const char* value,const char* table=nullptr)=0;

	// hash
	virtual int		hlen		(const char* key,const char* table=nullptr)=0;
	virtual int		hdel		(const char* key,const std::vector<std::string>& fields,const char* table=nullptr)=0;
	virtual bool	hexists		(const char* key,const char* field,const char* table=nullptr)=0;								//if exists field
	virtual bool	hset		(const char* key,const char* field,const char* value,const char* table=nullptr)=0;
	virtual bool	hsetnx		(const char* key,const char* field,const char* value,const char* table=nullptr)=0;
	virtual bool	hget		(const char* key,const char* field,std::string& value,const char* table=nullptr)=0;
	virtual void	hmset		(const char* key,const std::map<std::string,std::string>& map,const char* table=nullptr)=0;		//set multiple values
	virtual int		hmget		(const char* key,const std::vector<std::string>& fields,std::map<std::string,std::string>& map,const char* table=nullptr)=0;
	virtual int		hgetall		(const char* key,std::map<std::string,std::string>& map,const char* table=nullptr)=0;			//get all
	virtual int		hkeys		(const char* key,std::vector<std::string>& fields,const char* table=nullptr)=0;					//get all keys
	virtual int		hvals		(const char* key,std::vector<std::string>& vals,const char* table=nullptr)=0;					//get all values

	// set
	virtual int		sadd		(const char* key,const std::vector<std::string>& values,const char* table=nullptr)=0;
	virtual int		scard		(const char* key,const char* table=nullptr)=0;	//length
	virtual bool	sismember	(const char* key,const std::string& value,const char* table=nullptr)=0;
	virtual int		smembers	(const char* key,std::vector<std::string>& values,const char* table=nullptr)=0;
	virtual int		spop		(const char* key,int count,std::vector<std::string>& values,const char* table=nullptr)=0;
	virtual int		srem		(const char* key,const std::vector<std::string>& values,const char* table=nullptr)=0;			//remove

	// sort set
	virtual int		zadd		(const char* key,const std::map<std::string,int64_t>& map,const char* table=nullptr)=0;
	virtual int		zcard		(const char* key,const char* table=nullptr)=0;	//length
	virtual int		zcount		(const char* key,int64_t min,int64_t max,const char* table=nullptr)=0;		//count range
	virtual int		zrange		(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec,const char* table=nullptr)=0;
	virtual int		zrevrange	(const char* key,int start,int end,bool withscores,std::vector<std::pair<std::string,int64_t>>& vec,const char* table=nullptr)=0;
	virtual int		zrank		(const char* key,const char* field,const char* table=nullptr)=0;			//get rank
	virtual int		zrevrank	(const char* key,const char* field,const char* table=nullptr)=0;			//get reverse rank
	virtual long long zscore	(const char* key,const char* field,const char* table=nullptr)=0;			//get value
};
// --------------------------------------------------------
};// namespace
#endif // _vic_proxy_h_
