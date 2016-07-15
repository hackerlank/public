// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: db_proxy.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _db_proxy_h_
#define _db_proxy_h_

namespace keye{
// --------------------------------------------------------
// db_handler
// --------------------------------------------------------
class KEYE_API db_handler{
public:
	virtual			~db_handler(){}
};
// --------------------------------------------------------
// db_proxy:multi-thread async mysql proxy
// --------------------------------------------------------
class db_proxy_impl;
class KEYE_API db_proxy{
public:
					db_proxy(unsigned char threads=1);
	//open multi-connections to database
	bool			connect(const char* host,unsigned short port,const char* user,const char* passwd,const char* dbname);
};
// --------------------------------------------------------
};// namespace
#endif // _db_proxy_h_
