// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: work_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _wsWork_handler_h_
#define _wsWork_handler_h_

namespace keye{
	namespace ws{
// --------------------------------------------------------
// work handler:handle io events,called by service
// --------------------------------------------------------
class KEYE_API Work_handler{
public:
	virtual			~Work_handler(){}
	virtual void	on_open(sh_type&){}
	virtual void	on_close(sh_type&){}
	virtual void	on_read(sh_type&,void*,size_t){}
	virtual void	on_write(sh_type&,void*,size_t){}
	virtual void	on_event(sh_type&,void*,size_t){}
	virtual bool	on_timer(sh_type&,size_t id,size_t milliseconds){return true;}
};
// --------------------------------------------------------
};};
#endif //