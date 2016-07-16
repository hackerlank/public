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
// db_handler
// --------------------------------------------------------
class KEYE_API db_handler{
public:
	virtual			~db_handler(){}
};
// --------------------------------------------------------
// redis_proxy:multi-thread async mysql proxy
// --------------------------------------------------------
class redis_proxy_impl;
class KEYE_API redis_proxy{
public:
					redis_proxy(unsigned char threads=1);
	//open multi-connections to database
	bool			connect(const char* host,unsigned short port,const char* user,const char* passwd,const char* dbname);
};
// --------------------------------------------------------
};// namespace
#endif // _redis_proxy_h_
