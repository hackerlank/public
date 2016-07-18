// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_proxy.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_proxy_h_
#define _mysql_proxy_h_

namespace keye{
	class mysql_proxy_impl;
};
template class KEYE_API std::shared_ptr<keye::mysql_proxy_impl>;

namespace keye{
// --------------------------------------------------------
// mysql_handler
// --------------------------------------------------------
class KEYE_API mysql_handler{
public:
	virtual			~mysql_handler(){}
	virtual void	handle(mysql_resultset& resultset)=0;
};
// --------------------------------------------------------
// mysql_proxy:multi-thread async mysql proxy
// --------------------------------------------------------
class KEYE_API mysql_proxy{
public:
					mysql_proxy(unsigned char threads=1);
	//open multi-connections to database
	bool			connect(const char* host,unsigned short port,const char* user,const char* passwd,const char* dbname);
	//async execute
	void			execute(mysql_statement&,mysql_parameter*,mysql_handler* =nullptr);
private:
	s_ptr<mysql_proxy_impl>	_impl;
};
// --------------------------------------------------------
};// namespace
#endif // _mysql_proxy_h_
