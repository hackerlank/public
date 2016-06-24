// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: svc_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "wsio_fwd.h"
#include "impl_decl.hpp"

#ifdef _USE_LIBWEBSOCKET_

using namespace keye::ws;
// --------------------------------------------------------
Svc_handler_impl::Svc_handler_impl(struct libwebsocket_context& ctx,
		struct libwebsocket& wsi,Work_handler& w,wsio_alloc& a):
	_w(w),_a(a),
	_stopped(false),_id(-1),_port(0),
	_handler(*this),
	_wsi(wsi),
	_ctx(ctx){
		_id=libwebsocket_get_socket_fd(&wsi);
		char client_name[128];
		char client_ip[128];
		libwebsockets_get_peer_addresses(_id,client_name,
			sizeof(client_name), client_ip, sizeof(client_ip));
		_address=client_name;
		_port=atoi(client_ip);
}

size_t Svc_handler_impl::id()const{
	return _id;
}

void Svc_handler_impl::close(){
	libwebsocket_close_and_free_session(&_ctx,&_wsi,LWS_CLOSE_STATUS_NORMAL);
}

void Svc_handler_impl::send(void* buf,size_t length){
	//must make a copy,and deallocate after io service
	if(!_stopped){
//		if(void* wbuf=(void*)_a.allocate(length)){
//			memcpy(wbuf,buf,length);
			enum libwebsocket_write_protocol writeProtocol=LWS_WRITE_BINARY;
			libwebsocket_write(&_wsi,(unsigned char*)buf,length,writeProtocol);
//		}
	}
}

const char* Svc_handler_impl::address()const{
	return _address.c_str();
}

unsigned short Svc_handler_impl::port()const{
	return _port;
}
// --------------------------------------------------------
// svc_handler
// --------------------------------------------------------
svc_handler::svc_handler(Svc_handler_impl& i):_impl(i){}
size_t svc_handler::id()const				{return _impl.id();}
void svc_handler::close()					{_impl.close();}
void svc_handler::send(void* buf,size_t length)	{_impl.send(buf,length);}
const char* svc_handler::address()const		{return _impl.address();}
unsigned short svc_handler::port()const		{return _impl.port();}

#endif