// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_svc.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-8
 */
// --------------------------------------------------------
#ifndef _mysql_svc_h_
#define _mysql_svc_h_
#include <keye/htio/htio_fwd.h>
// --------------------------------------------------------
// mysql_svc
// --------------------------------------------------------
typedef std::allocator<char> alloc_type;	//we can override this allocator
namespace keye{
class mysql_svc;

class mysql_svc_handler{
public:
					mysql_svc_handler(mysql_svc& s);
	virtual void	on_event(svc_handler&,void*,size_t);
private:
	mysql_connection*	_conn;
	mysql_svc&			_svc;
};

class mysql_svc{
public:
				mysql_svc(mysql_proxy_impl* proxy,unsigned char threads=1)
					:_w(*this),_s(1,threads),_proxy(proxy){}
	void		start(){
		_s.run();
	}
	void		execute(mysql_statement& stmt,mysql_parameter* param,mysql_handler* h){
		//pack statement and parameter
		auto name=stmt.name();
		auto db=stmt.database();
		auto sz_name=name?strlen(name):0,
			sz_db=db?strlen(db):0;
		size_t sz=0,szppm=0;
		std::shared_ptr<char> sppm;
		if(param)sppm=param->deserialize(szppm);
		char* ppm=sppm.get();
		sz=sizeof(q_event)+sz_name+sz_db+2+sizeof(szppm)+szppm;
		auto buf=(char*)_a.allocate(sz);
		auto e=(q_event*)buf;
		e->handler=h;
		buf=(char*)e->buffer;
		buf_set(&buf,name,sz_name);	//name
		buf_set(&buf,'\0');
		buf_set(&buf,db,sz_db);		//db
		buf_set(&buf,'\0');
		buf_set(&buf,szppm);		//parameter
		if(ppm)buf_set(&buf,ppm,szppm);

		//post event
		_s.post_event(e,sz);
		_a.deallocate((char*)e,0);
	}
	mysql_connection*  get_connection(const char* db){
		mysql_connection* sp=nullptr;
		_mutex.lock();
		//boost::asio::detail::mutex::scoped_lock lock(_mutex);
		auto it=connections_.end(),iend=it;
		if(db)
			it=connections_.find(db);
		else if(!connections_.empty())
			it=connections_.begin();
		if(it!=iend){
			auto& conns=it->second.second;
			if(++it->second.first>=(int)conns.size())
				it->second.first=0;
			sp=conns[it->second.first].get();
		}
		_mutex.unlock();
		return sp;
	}
private:
	friend class mysql_proxy_impl;
	friend class mysql_svc_handler;
	mysql_svc_handler	_w;
	alloc_type			_a;
	service				_s;
	mysql_proxy_impl*	_proxy;
	//<db name<init index,connections>>,better using cycle pool
	std::mutex _mutex;
	//boost::asio::detail::mutex _mutex;
	std::map<std::string,std::pair<int,std::vector<s_ptr<mysql_connection>>>>	connections_;

	struct q_event{
		mysql_handler*	handler;
		char			buffer[0];
	};
};};

using namespace keye;
mysql_svc_handler::mysql_svc_handler(mysql_svc& s)
	 :_svc(s),_conn(nullptr){}

 void mysql_svc_handler::on_event(svc_handler&,void* data,size_t sz){
	auto e=(mysql_svc::q_event*)data;
	//unpack statement and parameter
	auto buf=(const char*)e->buffer;
	std::string name,db;
	buf_get(name,&buf);
	buf_get(db,&buf);
	if(!_conn)_conn=_svc.get_connection(db.empty()?nullptr:db.c_str());
	if(_conn){
		mysql_statement stmt(name.c_str(),db.c_str());
		size_t len=0;
		buf_get(len,&buf);
		if(len==0)buf=nullptr;
		mysql_parameter param((const void*)buf);
		if(auto res=_conn->execute(stmt,&param))
			if(e->handler)
				e->handler->handle(*res.get());
	}
}
#endif // _mysql_svc_h_
