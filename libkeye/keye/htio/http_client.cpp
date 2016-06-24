// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: http_client.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"

using namespace keye;
// --------------------------------------------------------
/*
class HttpSocket;
class HttpClientRunnable;
class WorldPacket;
class WorldSession;
*/
namespace keye{
class http_handler{
public:
					http_handler(http_client_impl& x):_cx(x){}
	virtual void	on_open(svc_handler& sh);
	virtual void	on_read(svc_handler& sh,void* buf,size_t sz);
	virtual bool	on_timer(svc_handler&,size_t id,size_t milliseconds);
private:
	http_client_impl&	_cx;
};

class http_client_impl{
public:
					http_client_impl();
	bool			connect(const char*,unsigned short);
	int				request(const char* =nullptr);
	void			set_responser(http_responser*);

	const char*		pop_request();
	void			handle(const char*);
private:
	void			_reconnect();
	void			_push_request(const char*);

	unsigned short	m_port;
	std::string		m_address;
	http_responser*	_responser;

	http_handler				_hx;
	service						_svc;

	//need a lock
	std::list<std::string>		_requests;
};};//namespace

http_client_impl::http_client_impl():
	m_port(0),
	_hx(*this),
	_svc(2,4,1460),
	_responser(nullptr){}

bool http_client_impl::connect(const char* host,unsigned short port){
	m_address=host;
	m_port=port;
	return true;
}

void http_client_impl::_reconnect(){
	_svc.connect(m_address.c_str(),m_port);
}

int http_client_impl::request(const char* query){
	_reconnect();
	_push_request(query);
	return 0;
}

void http_client_impl::handle(const char* buf){
	if(_responser)_responser->handle(buf);
}

void http_client_impl::set_responser(http_responser* r){
	_responser=r;;
}

const char* http_client_impl::pop_request(){
	return nullptr;
}

void http_client_impl::_push_request(const char*){
}
// --------------------------------------------------------
void http_handler::on_open(svc_handler& sh){
	if(auto query=_cx.pop_request()){
		const size_t buf_sz=4096;
		size_t len=0;
		unsigned char url[buf_sz];
		std::string address(sh.address());
		sprintf((char*)url,"GET %s HTTP/1.0\r\nHost:%s\r\nUser-Agent:Mozilla/4.0\r\n\r\n",query,address.c_str());
		len=strlen((const char*)url);
		sh.send(url,len);
	}
}

void http_handler::on_read(svc_handler& sh,void* buf,size_t sz){
	if(buf&&sz){
		std::string content((const char*)buf);
		std::string rn("\r\n"),rnrn("\r\n\r\n");
		auto ipos=content.find(rnrn);
		if(ipos!=std::string::npos)
			content=content.substr(ipos+rnrn.length());
		ipos=content.find(rn);
		if(ipos!=std::string::npos)
			content=content.substr(0,ipos);
		_cx.handle(content.c_str());
	}
}
bool http_handler::on_timer(svc_handler&,size_t id,size_t milliseconds){
	return true;
}
// --------------------------------------------------------
http_client::http_client(){
	_impl.reset(new http_client_impl());
}

bool http_client::connect(const char* host,unsigned short port){
	return _impl?_impl->connect(host,port):false;
}

int http_client::request(const char* str){
	return _impl?_impl->request(str):0;
}

void http_client::set_responser(http_responser* r){
	if(_impl)_impl->set_responser(r);
}
