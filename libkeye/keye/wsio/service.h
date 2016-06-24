// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: service.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _wsService_h_
#define _wsService_h_

#pragma warning(disable:4251)	//avoid export implementation class

NS_KEYE_WS_BEGIN
// use this as prototol name
static const char* protocol_name="keye-websocket";
// --------------------------------------------------------
/* service to process io,_W must be pass into,
	_W must derive from work_handler and it handles io event,
	_A must derice from io_alloc and it allocate for service,
	io runs on io threads,events and timer run on work threads,
	threads are auto chosen */
// --------------------------------------------------------
class Service_impl;
class KEYE_API Service{
public:
			Service(Work_handler& w,wsio_alloc& a,size_t ios=1,size_t works=4,size_t rb_size=1460);
	void	run(unsigned short port,const char* address=nullptr);
	void	connect(const char* address,unsigned short port,unsigned short conns=1);
	void	close();
	bool	closed()const;
private:
	std::shared_ptr<Service_impl>	_svc;
};

NS_KEYE_WS_END
#endif //