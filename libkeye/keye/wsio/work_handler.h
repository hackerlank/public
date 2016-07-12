// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: work_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _work_handler_h_
#define _work_handler_h_

namespace keye{
// --------------------------------------------------------
// work handler:handle io events,called by service
// --------------------------------------------------------
class KEYE_API work_handler{
public:
	virtual			~work_handler(){}
	virtual void	on_open(svc_handler&){}
	virtual void	on_close(svc_handler&){}
	virtual void	on_read(svc_handler&,void*,size_t){}
	virtual void	on_write(svc_handler&,void*,size_t){}
	virtual void	on_event(svc_handler&,void*,size_t){}
	virtual bool	on_timer(svc_handler&,size_t id,size_t milliseconds){return true;}
};
// --------------------------------------------------------
};
#endif // _work_handler_h_