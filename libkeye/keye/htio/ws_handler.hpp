// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_handler_impl.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _ws_handler_impl_hpp_
#define _ws_handler_impl_hpp_

using namespace keye;

class ws_handler_impl:public svc_handler{
	typedef service_type::connection_ptr service_handler_type;
public:
			ws_handler_impl(service_handler_type& sh):_sh(sh){}
	virtual void	close(){
		_sh->close(0, _rc);
	}
	virtual size_t	id()const{
		return (size_t)_sh->get_handle().lock().get();
	}
	virtual std::shared_ptr<svc_handler> operator()()const{
		return std::shared_ptr<svc_handler>(new ws_handler_impl(_sh));
	}
	virtual void	send(void* buf,size_t length){
		_sh->send((const void*)buf, length, websocketpp::frame::opcode::value::binary);
	}
	virtual void	post_event(void* buf,size_t length){
		//_sh.post_event(buf,length);
	}
	virtual void	set_timer(size_t id,size_t milliseconds){
		//unset_timer(id);
		//timers_[id] = _sh->set_timer((long)milliseconds, std::bind(&ws_handler_impl::on_timer, this, id, milliseconds, std::placeholders::_1));
	}
	virtual void	unset_timer(size_t id){
		auto i = timers_.find(id);
		if (i != timers_.end())
			i->second->cancel();
	}
	virtual std::string	address()const{
		try{
            return _sh->get_uri()->str();
			//return _sh->get_host();
		} catch(...){
			return "0.0.0.0";
		}
	}
	virtual unsigned short	port()const{
		try{
			return _sh->get_port();
		} catch(...){
			return 0;
		}
	}
	virtual std::shared_ptr<void>&	sptr(){
		return _s_ptr;
	}
	void on_timer(size_t id, size_t milliseconds, websocketpp::lib::error_code const & ec) {
		if(!ec)
			// set timer for next telemetry check
			set_timer(id, milliseconds);
	}
private:
	service_handler_type&	_sh;
	const std::string		_rc;
	///the interface expsure
	std::shared_ptr<void>	_s_ptr;
	/// Events timers
	std::map<std::size_t, service_type::timer_ptr> timers_;
};
// --------------------------------------------------------
#endif
