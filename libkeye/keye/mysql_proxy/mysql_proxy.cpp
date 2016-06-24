// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: Receiver.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2011-4-22
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/mysql_proxy/mysql_proxy_fwd.h>
#include <keye/mysql_proxy/mysql_statement.hpp>
#include <keye/mysql_proxy/mysql_connection.hpp>

//#define _PROXY_SVC_

namespace keye{
class mysql_svc;
class mysql_proxy_impl{
public:
						mysql_proxy_impl(unsigned char conns=1);
	bool				connect(const char* host, uint32_t port,
							const char* user, const char* passwd,
							const char* dbname);
	void				execute(mysql_statement&,mysql_parameter*,mysql_handler* =nullptr);
private:
	friend class mysql_svc_handler;
	mysql_connection*	_connect(const char* host, uint32_t port,
							const char* user, const char* passwd,
							const char* dbname);
#ifdef _PROXY_SVC_
	s_ptr<mysql_svc>	_svc;
	unsigned char		_threads;
#else
	s_ptr<mysql_connection>	_conn;
#endif
};
};
#ifdef _PROXY_SVC_
#include <keye/mysql_proxy/mysql_svc.hpp>
#endif

using namespace keye;

mysql_proxy_impl::mysql_proxy_impl(unsigned char conns)
#ifdef _PROXY_SVC_
	:_threads(conns){
		_svc.reset(new mysql_svc(this,conns));
		_svc->start();
#else
	{
#endif
}

bool mysql_proxy_impl::connect(const char* host, uint32_t port,
		const char* user, const char* passwd,const char* db){
#ifdef _PROXY_SVC_
	auto& conns=_svc->connections_;
	auto it=conns.find(db),iend=conns.end();
	if(it!=iend)
		return true;

	std::vector<s_ptr<mysql_connection>> v;
	for(auto i=_threads;i;--i){
		if(auto c=_connect(host,port,user,passwd,db)){
			s_ptr<mysql_connection> spc(c);
			v.push_back(spc);
		}
	}
	if(v.empty()){
		return false;
	}else{
		conns.insert(std::make_pair(db,std::make_pair(0,v)));
		return true;
	}
#else
	if(!_conn)_conn.reset(_connect(host,port,user,passwd,db));
	return _conn.get()!=nullptr;
#endif
}

mysql_connection* mysql_proxy_impl::_connect(const char* host, uint32_t port,
		const char* user, const char* passwd,const char* db){
	MYSQL* mysql=nullptr;
	do{
		mysql=mysql_init(NULL);
		//mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		unsigned int connect_timeout = 30;
		mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&connect_timeout);
		mysql_options(mysql,MYSQL_OPT_RECONNECT,"1");
		//avoid reporting data trunction when mysql_stmt_fetch()
		my_bool attr=0;
		mysql_options(mysql, MYSQL_REPORT_DATA_TRUNCATION, &attr);

		unsigned long flags = 0;
		flags |= CLIENT_MULTI_RESULTS;
		if ( ! mysql_real_connect(mysql, host, user, passwd, db, port, NULL, flags)){
			KEYE_LOG("mysql_real_connect failed, %s",mysql_error(mysql));
			break;
		}
		const char* state = mysql_stat(mysql);
		if (state == NULL){
			KEYE_LOG("mysql_stat failed, %s",mysql_error(mysql));
			break;
		}

		return new mysql_connection(mysql);
	}while(false);
	if(mysql)mysql_close(mysql);
	return nullptr;
}

void mysql_proxy_impl::execute(mysql_statement& stmt,mysql_parameter* param,mysql_handler* h){
#ifdef _PROXY_SVC_
	_svc->execute(stmt,param,h);
#else
	if(_conn){
		auto res=_conn->execute(stmt,param);
		if(h)h->handle(*res.get());
	}
#endif
}
// --------------------------------------------------------
// mysql_proxy
// --------------------------------------------------------
mysql_proxy::mysql_proxy(unsigned char conns){
	_impl.reset(new mysql_proxy_impl(conns));
}

bool mysql_proxy::connect(const char* host, unsigned short port,
		const char* user, const char* passwd,const char* db){
	return _impl?_impl->connect(host,port,user,passwd,db):false;
}

void mysql_proxy::execute(mysql_statement& stmt,mysql_parameter* param,mysql_handler* h){
	if(_impl)_impl->execute(stmt,param,h);
}
