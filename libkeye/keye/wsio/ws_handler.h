// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _ws_handler_h_
#define _ws_handler_h_

namespace keye{
// --------------------------------------------------------
/* service handler:object for handle socket operations
	svc_handler is created by and pass to service,
	svc_handler is only the interface,htio encapse the implements */
// --------------------------------------------------------
class KEYE_API ws_handler{
public:
	virtual					~ws_handler(){}
	virtual void			close()=0;
	virtual size_t			id()const=0;
	//clone
	virtual std::shared_ptr<ws_handler>
							operator()()const=0;
	//io method
	//send,post_event:buf must without length data
	virtual void			send(void* buf,size_t length)=0;
	virtual void			post_event(void* buf,size_t length)=0;
	virtual void			set_timer(size_t id,size_t milliseconds)=0;
	virtual void			unset_timer(size_t id)=0;
	//address and port
	virtual std::string		address()const=0;
	virtual unsigned short	port()const=0;
	//the interface expsure
	virtual std::shared_ptr<void>&
							sptr()=0;
};
// --------------------------------------------------------
};
#endif // _ws_handler_h_