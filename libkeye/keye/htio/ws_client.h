// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_client.h
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _ws_client_h_
#define _ws_client_h_

#pragma warning(disable:4251)	//avoid export implementation class

namespace keye{
// --------------------------------------------------------
/* ws_client to process io,
	io runs on io threads,events and timer run on work threads,
	threads are auto chosen */
	// --------------------------------------------------------
class ws_client_impl;

class KEYE_API ws_client{
public:
	//ios:io threads;works:work threads;rb_size:read buffer max size
			ws_client(size_t ios=1,size_t works=1,size_t rb_size=510);
	virtual	~ws_client(){close();}
	//run as server,we do not open accept while port was 0
	void	run(unsigned short port=0,const char* address=nullptr);
	//run as client and connect to server
	void	connect(const char* uri,unsigned short conns=1);
	void	close();
	bool	closed()const;
	//set up a timer on work thread,id is overlayable
	void	set_timer(size_t id,size_t milliseconds);
	void	unset_timer(size_t id);
	//post event to work thead
	void	post_event(void* buf,size_t length);

	//events handlers
	virtual void	on_open(svc_handler&){}
	virtual void	on_close(svc_handler&){}
	virtual void	on_read(svc_handler&,void*,size_t){}
	virtual void	on_write(svc_handler&,void*,size_t){}
	virtual void	on_event(svc_handler&,void*,size_t){}
	virtual bool	on_timer(svc_handler&,size_t id,size_t milliseconds){ return true; }
private:
	std::shared_ptr<ws_client_impl>	_svc;
};
// --------------------------------------------------------
};
#endif // _ws_client_h_