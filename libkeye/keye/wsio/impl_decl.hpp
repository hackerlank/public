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

namespace keye{
	namespace ws{
// --------------------------------------------------------
// Svc_handler_impl
// --------------------------------------------------------
class Svc_handler_impl{
public:
					Svc_handler_impl(struct libwebsocket_context&,struct libwebsocket&,
						Work_handler& w,wsio_alloc& a);
	size_t			id()const;
	void			close();
	//io method
	void			send(void* buf,size_t length);
	//address and port
	const char*		address()const;
	unsigned short	port()const;
private:
	friend class Service_impl;

	int				_id;
	bool			_stopped;
	unsigned short	_port;
	std::string		_address;
	Work_handler&	_w;
	wsio_alloc&		_a;

	struct libwebsocket_context&	_ctx;
	struct libwebsocket&			_wsi;
	svc_handler				_handler;
};
typedef std::shared_ptr<Svc_handler_impl>	sh_sptr;
// --------------------------------------------------------
};};
#endif // _impl_decl_h_