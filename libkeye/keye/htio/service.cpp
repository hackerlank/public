// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ICore.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"
// --------------------------------------------------------
// service_impl: service->service_impl->bas::service
//					|						 |
//					 <--work_handler_impl<---
// --------------------------------------------------------
typedef std::allocator<char> alloc_type;	//we can override this allocator
#include "svc_handler.hpp"
#include "work_handler.hpp"
namespace keye{
class service_impl{
public:
	service_impl(service& w,size_t ios,size_t works,size_t rb_size)
	:_w(w),_ios(ios),_works(works),_rb_size(rb_size),_bExit(true){}

	void	run(unsigned short port,const char* address=nullptr){
		_bExit=false;
		if(!service_)
			service_.reset(new service_type(&_w,&_a,_ios,_works,_rb_size));
		//start service thread
		_thread.reset(new std::thread(boost::bind(&service_type::run,service_.get(),port,address)));
	}
	void	close(){
		if(!_bExit){
			if(service_)
				service_->stop();
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
		if(!service_)
			service_.reset(new service_type(&_w,&_a,_ios,_works,_rb_size));
		service_->connect(address,port,conns);
	}
	void	set_timer(size_t id,size_t milliseconds){
		if(service_)service_->set_timer(id,milliseconds);
	}
	void	unset_timer(size_t id){
		if(service_)service_->unset_timer(id);
	}
	void	post_event(void* buf,size_t length){
		if(service_)service_->post_event(buf,length);
	}
private:
	typedef bas::service<work_handler_impl,alloc_type> service_type;
	work_handler_impl	_w;
	alloc_type			_a;
	size_t				_ios,_works,_rb_size;
	std::shared_ptr<service_type>	service_;
	std::shared_ptr<std::thread>	_thread;
	bool							_bExit;
};};

using namespace keye;
// --------------------------------------------------------
// service
// --------------------------------------------------------
service::service(size_t ios,size_t works,size_t rb_size){
	_svc.reset(new service_impl(*this,ios,works,rb_size));
}

void service::connect(const char* address,unsigned short port,unsigned short conns){
	if(_svc)_svc->connect(address,port,conns);
}
void service::run(unsigned short port,const char* address){
	if(_svc)_svc->run(port,address);
}
void service::close(){
	if(_svc)_svc->close();
}
bool service::closed()const{
	return _svc?_svc->closed():true;
}
void service::set_timer(size_t id,size_t milliseconds){
	if(_svc)_svc->set_timer(id,milliseconds);
}

void service::unset_timer(size_t id){
	if(_svc)_svc->unset_timer(id);
}

void service::post_event(void* buf,size_t length){
	if(_svc)_svc->post_event(buf,length);
}

