// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_service.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "wsio_fwd.h"
// --------------------------------------------------------
typedef std::allocator<char> alloc_type;	//we can override this allocator
#include "ws_handler.hpp"
#include "work_handler.hpp"
namespace keye{
class ws_service_impl{
public:
	ws_service_impl(ws_service& w,size_t ios,size_t works,size_t rb_size)
	:_w(w),_ios(ios),_works(works),_rb_size(rb_size),_bExit(true){}

	void	run(unsigned short port,const char* address=nullptr){
		_bExit=false;
		if(!ws_service_)
			ws_service_.reset(new ws_service_type(&_w,&_a,_ios,_works,_rb_size));
		//start ws_service thread
		_thread.reset(new std::thread(boost::bind(&ws_service_type::run,ws_service_.get(),port,address)));
	}
	void	close(){
		if(!_bExit){
			if(ws_service_)
				ws_service_->stop();
			if(_thread){
				_thread->join();
				_thread.reset();
			}
			_bExit=true;
		}
	}
	bool	closed()const{
		return _bExit;
	}
	void	connect(const char* address,unsigned short port,unsigned short conns=1){
		_bExit=false;
		if(!ws_service_)
			ws_service_.reset(new ws_service_type(&_w,&_a,_ios,_works,_rb_size));
		ws_service_->connect(address,port,conns);
	}
	void	set_timer(size_t id,size_t milliseconds){
		if(ws_service_)ws_service_->set_timer(id,milliseconds);
	}
	void	unset_timer(size_t id){
		if(ws_service_)ws_service_->unset_timer(id);
	}
	void	post_event(void* buf,size_t length){
		if(ws_service_)ws_service_->post_event(buf,length);
	}
private:
	typedef bas::ws_service<work_handler_impl,alloc_type> ws_service_type;
	work_handler_impl	_w;
	alloc_type			_a;
	size_t				_ios,_works,_rb_size;
	std::shared_ptr<ws_service_type>	ws_service_;
	std::shared_ptr<std::thread>	_thread;
	bool							_bExit;
};

// --------------------------------------------------------
// ws_service
// --------------------------------------------------------
ws_service::ws_service(size_t ios,size_t works,size_t rb_size){
	_svc.reset(new ws_service_impl(*this,ios,works,rb_size));
}

void ws_service::connect(const char* address,unsigned short port,unsigned short conns){
	if(_svc)_svc->connect(address,port,conns);
}
void ws_service::run(unsigned short port,const char* address){
	if(_svc)_svc->run(port,address);
}
void ws_service::close(){
	if(_svc)_svc->close();
}
bool ws_service::closed()const{
	return _svc?_svc->closed():true;
}
void ws_service::set_timer(size_t id,size_t milliseconds){
	if(_svc)_svc->set_timer(id,milliseconds);
}

void ws_service::unset_timer(size_t id){
	if(_svc)_svc->unset_timer(id);
}

void ws_service::post_event(void* buf,size_t length){
	if(_svc)_svc->post_event(buf,length);
}
};
