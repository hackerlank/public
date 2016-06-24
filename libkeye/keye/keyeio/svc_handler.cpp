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
#include "keyeio_fwd.h"
#include "impl_decl.hpp"

using namespace boost::asio;
// --------------------------------------------------------
using namespace keye;
Svc_handler_impl::Svc_handler_impl(Work_handler& w,io_alloc& a,
		io_service& ios_io,io_service& ios_work,
		size_t rb_size):
	_w(w),_a(a),
	_ios_io(ios_io),_ios_work(ios_work),
	_rb_size(rb_size),
	_stopped(false),_id(-1),_port(0),
	_read_ptr(nullptr),
	_handler(*this){
		_sock.reset(new socket_type(_ios_io));
}

size_t Svc_handler_impl::id()const{
	return _id;
}

void Svc_handler_impl::close(){
	boost::system::error_code ignored_ec;
	_close(ignored_ec);
}

void Svc_handler_impl::send(void* buf,size_t length){
	//must make a copy,and deallocate after io service
	if(!_stopped)
	if(void* wbuf=(void*)_a.allocate(length)){
		memcpy(wbuf,buf,length);
		auto b=buffer(wbuf,length);
		_ios_io.dispatch(boost::bind(&Svc_handler_impl::_async_write_i<decltype(b)>,
		shared_from_this(),b));
	}
}

/// Start an asynchronous operation from io_service thread to write buffers to the socket.
template<typename Buffers> void Svc_handler_impl::_async_write_i(Buffers& buffers){
	auto wbuf=boost::asio::detail::buffer_cast_helper(buffers);
	// The handler has been stopped,break.
	if(_stopped)
		_a.deallocate((void*)wbuf,1);
	else
		boost::asio::async_write(_socket(),
			buffers,
			boost::bind(&Svc_handler_impl::_handle_write,
			shared_from_this(),
			placeholders::error,
			placeholders::bytes_transferred,wbuf));
}

/// Handle completion of a write operation in io_service thread.
void Svc_handler_impl::_handle_write(const boost::system::error_code& e,
		std::size_t bytes_transferred,void* wbuf){
	// The handler has been stopped,break.
	if(_stopped||e){
		_a.deallocate(wbuf,1);
		// Close with error_code e.
		_close(e);
	}else{
		// Post to work_service for executing do_write.
		_ios_work.post(boost::bind(&Svc_handler_impl::_do_write,
			shared_from_this(),
			bytes_transferred,wbuf));
	}
}

/// Do on_write in work_service thread.
void Svc_handler_impl::_do_write(std::size_t bytes_transferred,void* wbuf){
	if(!_stopped)
		// Call on_write function of the work handler.
		_w.on_write(_handler,wbuf,bytes_transferred);
	if(wbuf)
		_a.deallocate((void*)wbuf,1);
}

void Svc_handler_impl::post_event(void* buf,size_t length){
}

/// Setup a timer.
void Svc_handler_impl::set_timer(size_t id,size_t milliseconds){
	// Don't set timer while handler closed.
	if(_stopped)return;

	std::map<std::size_t,timer_ptr>::iterator i=_timers.find(id);
	timer_ptr timer;
	if(_timers.end()==i){
		//using work io_service
		timer.reset(new boost::asio::deadline_timer(_ios_work));
		_timers.insert(std::make_pair(id,timer));
	}else
		timer=i->second;
	if(timer){
		timer->expires_from_now(boost::posix_time::milliseconds(milliseconds));
		timer->async_wait(boost::bind(&Svc_handler_impl::_handle_timer,
		shared_from_this(),
		placeholders::error,id,milliseconds));
	}
}

/// Unset timer.
void Svc_handler_impl::unset_timer(size_t id){
	std::map<std::size_t,timer_ptr>::iterator i=_timers.find(id);
	if(_timers.end()!=i){
		timer_ptr timer=i->second;
		timer->cancel();
		_timers.erase(id);
	}
}

/// Handle timeout of whole operation in io_service thread.
void Svc_handler_impl::_handle_timer(const boost::system::error_code& e,std::size_t id,std::size_t milliseconds){
	if(!e){
		//break timer while false on_timer
		if(_w.on_timer(_handler,id,milliseconds))
			set_timer(id,milliseconds);
	}else{
	//		unset_timer(id);
	}
}

const char* Svc_handler_impl::address()const{
	return _address.c_str();
}

unsigned short Svc_handler_impl::port()const{
	return _port;
}

socket_type& Svc_handler_impl::_socket(){
	return *_sock;
}

void Svc_handler_impl::_close(const boost::system::error_code& e){
	_stopped=true;
	// Initiate graceful service_handler closure.
	_id=-1;
	_socket().lowest_layer().shutdown(ip::tcp::socket::shutdown_both,(boost::system::error_code&)e);
	_socket().lowest_layer().close();
}

void Svc_handler_impl::_open(){
	// save id and address
	_id=(int)_sock->native_handle();

	ip::tcp::endpoint local_endpoint=_socket().local_endpoint(),
		remote_endpoint=_socket().remote_endpoint();
	ip::address address=remote_endpoint.address();
	_port=remote_endpoint.port();
	_address=address.to_string();
	// notify
	_ios_work.post(boost::bind(&Svc_handler_impl::_do_open,shared_from_this()));
	_stopped=false;
}

/// Do on_open in work_service thread.
void Svc_handler_impl::_do_open(){
	// The handler has been stopped,do nothing.
	if(_stopped)return;
	// Call on_open function of the work handler.
	_w.on_open(_handler);
	_async_read_some();
}

void Svc_handler_impl::_async_read_some(){
	if(_read_ptr=(void*)_a.allocate(_rb_size)){
		auto b=buffer(_read_ptr,_rb_size);
		_ios_io.dispatch(boost::bind(&Svc_handler_impl::_async_read_some_i<decltype(b)>,shared_from_this(),b));
	}
}

/// Start an asynchronous operation from io_service thread to read any amount of data to buffers from the socket.
template<typename Buffers>
void Svc_handler_impl::_async_read_some_i(Buffers& buffers){
	// The handler has been stopped,do nothing.
	if(_stopped)return;
	_socket().async_read_some(buffers,
		boost::bind(&Svc_handler_impl::_handle_read,
			shared_from_this(),
			placeholders::error,
			placeholders::bytes_transferred,
			detail::buffer_cast_helper(buffers)));
}

void Svc_handler_impl::_handle_read(const boost::system::error_code& e,std::size_t bytes_transferred,void* buf){
	// The handler has been stopped,do nothing.
	if(_stopped)return;

	if(!e){
		// Post to work_service for executing _do_read.
		_ios_work.post(boost::bind(&Svc_handler_impl::_do_read,shared_from_this(),bytes_transferred,buf));
		_async_read_some();
	}else{
		// Close with error_code e.
		_close(e);
		// Post to work_service to executing do_close.
		_ios_work.post(boost::bind(&Svc_handler_impl::_do_close,shared_from_this(),e));
	}
}

/// Do on_read in work_service thread.
void Svc_handler_impl::_do_read(std::size_t bytes_transferred,void* rbuf){
	// The handler has been stopped,do nothing.
	if(!_stopped&&rbuf&&bytes_transferred)
		_w.on_read(_handler,rbuf,bytes_transferred);
	if(rbuf){
		_a.deallocate((void*)rbuf,1);
		//		read_ptr_=nullptr;
	}
}

void Svc_handler_impl::_connect(ip::tcp::endpoint& endpoint){
	_socket().lowest_layer().async_connect(endpoint,
		boost::bind(&Svc_handler_impl::_handle_connect,
		shared_from_this(),
		placeholders::error));
}

/// Handle completion of a connect operation in io_service thread.
void Svc_handler_impl::_handle_connect(const boost::system::error_code& e){
	// The handler has been stopped,do nothing.
	if(!e){
		_open();
		KEYE_LOG("connected to %s:%d.\n",_address.c_str(),_port);
	}else
		// Close with error_code e.
		_close(e);
}

/// Do on_close and reset handler for next connaction in work_service thread.
void Svc_handler_impl::_do_close(const boost::system::error_code& e){
	// Call on_close function of the work handler.
	_w.on_close(_handler);
	// Leave socket to destroy delay for finishing uncompleted SSL operations.
	// Leave io_service_/work_service_ for finishing uncompleted operations.
}
// --------------------------------------------------------
// svc_handler
// --------------------------------------------------------
Svc_handler::Svc_handler(Svc_handler_impl& i):_impl(i){}
size_t Svc_handler::id()const		{return _impl.id();}
void Svc_handler::close()		{_impl.close();}
void Svc_handler::send(void* buf,size_t length)				{_impl.send(buf,length);}
void Svc_handler::post_event(void* buf,size_t length)		{_impl.close();}
void Svc_handler::set_timer(size_t id,size_t milliseconds)	{_impl.close();}
void Svc_handler::unset_timer(size_t id)					{_impl.close();}
const char* Svc_handler::address()const		{return _impl.address();}
unsigned short Svc_handler::port()const		{return _impl.port();}
