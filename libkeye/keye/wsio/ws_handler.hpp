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
class work_handler_impl;

class ws_handler_impl:public ws_handler{
	typedef bas::service_handler<work_handler_impl,alloc_type> service_handler_type;
public:
			ws_handler_impl(service_handler_type& sh):_sh(sh){}
	virtual void	close(){
		_sh.close();
	}
	virtual size_t	id()const{
		return (size_t)&_sh;
	}
	virtual std::shared_ptr<ws_handler> operator()()const{
		return std::shared_ptr<ws_handler>(new ws_handler_impl(_sh));
	}
	virtual void	send(void* buf,size_t length){
		_sh.async_write(buf,length);
	}
	virtual void	post_event(void* buf,size_t length){
		_sh.post_event(buf,length);
	}
	virtual void	set_timer(size_t id,size_t milliseconds){
		_sh.set_timer(id,milliseconds);
	}
	virtual void	unset_timer(size_t id){
		_sh.unset_timer(id);
	}
	virtual std::string	address()const{
		try{
			boost::asio::ip::tcp::endpoint remote_endpoint=_sh.socket().remote_endpoint();
			boost::asio::ip::address address=remote_endpoint.address();
			return address.to_string();
		} catch(...){
			return "0.0.0.0";
		}
	}
	virtual unsigned short	port()const{
		try{
			boost::asio::ip::tcp::endpoint remote_endpoint=_sh.socket().remote_endpoint();
			return remote_endpoint.port();
		} catch(...){
			return 0;
		}
	}
	virtual std::shared_ptr<void>&	sptr(){
		return _sh.sptr();
	}
private:
	service_handler_type&	_sh;
};
// --------------------------------------------------------
#endif
