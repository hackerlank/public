// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: Svc_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _Svc_handler_h_
#define _Svc_handler_h_

namespace keye{
// --------------------------------------------------------
/* service handler:object for handle socket operations
	Svc_handler is created by service and pass to work_handler,
	Svc_handler is only the interface,htio encapse the implements */
// --------------------------------------------------------
class Svc_handler_impl;
class KEYE_API Svc_handler{
public:
					Svc_handler(Svc_handler_impl&);
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
	Svc_handler_impl&	_impl;
	//the interface expsure
//	std::shared_ptr<Svc_handler_impl>	_impl;
};
typedef Svc_handler		sh_type;
// --------------------------------------------------------
};
#endif // _svc_handler_h_