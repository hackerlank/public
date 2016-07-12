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
#ifndef _wsSvc_handler_h_
#define _wsSvc_handler_h_

namespace keye{
	namespace ws{
// --------------------------------------------------------
/* service handler:object for handle socket operations
	Svc_handler is created by service and pass to work_handler,
	Svc_handler is only the interface,htio encapse the implements */
// --------------------------------------------------------
class Svc_handler_impl;
class KEYE_API svc_handler{
public:
					svc_handler(Svc_handler_impl&);
	size_t			id()const;
	void			close();
	//io method
	//send,post_event:buf must without length data
	void			send(void* buf,size_t length);
	//address and port
	const char*		address()const;
	unsigned short	port()const;
private:
	Svc_handler_impl&	_impl;
};
typedef svc_handler		sh_type;
// --------------------------------------------------------
};};
#endif //