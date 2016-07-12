// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: work_handler_impl.h
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _work_handler_impl_hpp_
#define _work_handler_impl_hpp_

class work_handler_impl{
	typedef bas::service_handler<work_handler_impl,alloc_type> service_handler_type;
public:
			work_handler_impl(service& base):_base(base){}
	void	on_open(service_handler_type& handler){
		svc_handler_impl sh(handler);
		_base.on_open(sh);
	}
	void	on_close(service_handler_type& handler,const boost::system::error_code& e){
		svc_handler_impl sh(handler);
		_base.on_close(sh);
	}
	void	on_read(service_handler_type& handler,void* buf,std::size_t sz){
		svc_handler_impl sh(handler);
		_base.on_read(sh,buf,sz);
	}
	void	on_write(service_handler_type& handler,std::size_t sz,void* buf=nullptr){
		svc_handler_impl sh(handler);
		_base.on_write(sh,buf,sz);
	}
	void	on_event(service_handler_type& handler,std::size_t sz,void* buf=nullptr){
		svc_handler_impl sh(handler);
		_base.on_event(sh,buf,sz);
	}
	bool	on_timer(service_handler_type& handler,size_t id,size_t milliseconds){
		svc_handler_impl sh(handler);
		return _base.on_timer(sh,id,milliseconds);
	}
private:
	service&	_base;
};
// --------------------------------------------------------
#endif