// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: impl_decl.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _impl_decl_h_
#define _impl_decl_h_

using namespace boost::asio;

typedef ip::tcp::socket socket_type;

namespace keye{
// --------------------------------------------------------
// Svc_handler_impl
// --------------------------------------------------------
class Svc_handler_impl:public std::enable_shared_from_this<Svc_handler_impl>{
public:
					using std::enable_shared_from_this<Svc_handler_impl>::shared_from_this;
					Svc_handler_impl(Work_handler& w,io_alloc& a,
						io_service& ios_io,io_service& ios_work,
						size_t rb_size=1460);
	size_t			id()const;
	void			close();
	//io method
	//send,post_event:buf must without length data
	void			send(void* buf,size_t length);
	void			post_event(void* buf,size_t length);
	void			set_timer(size_t id,size_t milliseconds);
	void			unset_timer(size_t id);
	//address and port
	const char*		address()const;
	unsigned short	port()const;
private:
	friend class Service_impl;
	socket_type&	_socket();
	void			_connect(ip::tcp::endpoint& endpoint);
	void			_close(const boost::system::error_code&);

	void			_open();
	void			_handle_connect(const boost::system::error_code& e);
	void			_async_read_some();
					template<typename Buffers>
	void			_async_read_some_i(Buffers&);
	void			_handle_read(const boost::system::error_code& e,std::size_t bytes_transferred,void* buf);
					template<typename Buffers>
	void			_async_write_i(Buffers& buffers);
	void			_handle_write(const boost::system::error_code& e,std::size_t bytes_transferred,void* wbuf);
	void			_do_write(const std::size_t bytes_transferred,void* wbuf);
	void			_handle_timer(const boost::system::error_code& e,std::size_t id,std::size_t milliseconds);

	/// work thread
	void			_do_open();
	void			_do_read(std::size_t bytes_transferred,void* rbuf);
	void			_do_close(const boost::system::error_code& e);

	int				_id;
	bool			_stopped;
	unsigned short	_port;
	std::string		_address;
	//ios
	io_service		&_ios_io,&_ios_work;
	Work_handler&	_w;
	io_alloc&		_a;
	size_t			_rb_size;
	void*			_read_ptr;

	Svc_handler		_handler;
	/// Socket for the service_handler.
	std::shared_ptr<socket_type>	_sock;
	/// Events timers
	typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;
	std::map<std::size_t,timer_ptr> _timers;
};
typedef std::shared_ptr<Svc_handler_impl>	sh_sptr;
// --------------------------------------------------------
};
#endif // _impl_decl_h_