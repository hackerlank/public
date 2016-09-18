// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: svc_handler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _svc_handler_h_
#define _svc_handler_h_

namespace keye{
// --------------------------------------------------------
/* service handler:object for handle socket operations
	svc_handler is created by and pass to service,
	svc_handler is only the interface,htio encapse the implements */
// --------------------------------------------------------
class KEYE_API svc_handler{
public:
	virtual					~svc_handler(){}
    virtual void			close(){}
	virtual size_t			id()const=0;
	//clone
	virtual std::shared_ptr<svc_handler>
							operator()()const=0;
	//io method
	//send,post_event:buf must without length data
	virtual void			send(void* buf,size_t length)=0;
    virtual void			post_event(void* buf,size_t length){}
    virtual void			set_timer(size_t id,size_t milliseconds){}
    virtual void			unset_timer(size_t id){}
	//address and port
	virtual std::string		address()const=0;
	virtual unsigned short	port()const=0;
	//the interface expsure
    virtual std::shared_ptr<void> sptr(){return std::make_shared<char>();}
};
// --------------------------------------------------------
// null svc_handler
// --------------------------------------------------------
class null_svc_handler:public svc_handler{
public:
    virtual size_t	id()const{return 0;}
    virtual std::shared_ptr<svc_handler> operator()()const{
        return std::make_shared<null_svc_handler>();
    }
    virtual void	send(void* buf,size_t length){
    }
    virtual std::string	address()const{
        return "0.0.0.0";
    }
    virtual unsigned short	port()const{
        return 0;
    }
};
// --------------------------------------------------------
};
#endif // _svc_handler_h_