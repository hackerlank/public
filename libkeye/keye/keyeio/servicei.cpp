// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: service.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "keyeio_fwd.h"
#include "impl_decl.hpp"

using namespace boost::asio;
// --------------------------------------------------------
// Service_impl
// --------------------------------------------------------
namespace keye{
class Service_impl:public std::enable_shared_from_this<Service_impl>{
public:
			using std::enable_shared_from_this<Service_impl>::shared_from_this;
			Service_impl(Work_handler& w,io_alloc& a,size_t ios,size_t works,size_t rb_size)
				:_w(w),_a(a),_ios(ios),_works(works),_rb_size(rb_size),_started(false)
				,_acceptor(_ios_io){}
	void	run(unsigned short port,const char* address=nullptr);
	void	connect(const char* address,unsigned short port,unsigned short conns=1);
	void	close();
	bool	closed()const;
	void	set_timer(size_t id,size_t milliseconds);
	void	unset_timer(size_t id);
	void	post_event(void* buf,size_t length);

	void	handle_accept(const boost::system::error_code& e,sh_sptr& handler);
private:
	/// trigger
	void	_accept_one();

	int		_set_rlimit();
	void	_set_sig();
	template<class _SVC>
	void	_start_threads(int);

	Work_handler&	_w;
	io_alloc&		_a;
	size_t			_ios,_works,_rb_size;
	bool			_started;

	typedef std::shared_ptr<io_service::work>	work_ptr;
	typedef std::shared_ptr<std::thread>		thread_ptr;
	std::vector<work_ptr>	_work;
	std::vector<thread_ptr>	_threads;
	sh_sptr			_timer_handler;
	/// The io io_service
	io_service		_ios_io,_ios_work;
	/// The acceptor used to listen for incoming connections.
	ip::tcp::acceptor _acceptor;
	/// The server endpoint.
	ip::tcp::endpoint _endpoint;
};};
// --------------------------------------------------------
using namespace keye;

void io_svc(){};
void Service_impl::run(unsigned short port,const char* address){
    if(_started)return;

	//io threads
	if(port){
		//we do not open accept while port was 0
		if(address&&0<strlen(address))
			//auto address
			_endpoint.address(ip::address::from_string(address));
		_endpoint.port(port);

		// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
		_acceptor.open(_endpoint.protocol());
		_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
		try{
			_acceptor.bind(_endpoint);
			_acceptor.listen();
		}catch(boost::system::system_error se){
			KEYE_LOG(se.what());
		}
		  // Accept new connection.
		_accept_one();

		_timer_handler.reset(new Svc_handler_impl(_w,_a,_ios_io,_ios_work,_rb_size));
		// Give the io_service work to do so that its run() functions will not
		// exit until work was explicitly destroyed.
		_work.push_back(work_ptr(new io_service::work(_ios_io)));
		_work.push_back(work_ptr(new io_service::work(_ios_work)));

		// Create a thread to run the io_service.
		_threads.push_back(thread_ptr(new std::thread(boost::bind(&io_service::run,&_ios_io))));
		_threads.push_back(thread_ptr(new std::thread(boost::bind(&io_service::run,&_ios_work))));
		_started=true;

		auto host=_endpoint.address().to_string();
		KEYE_LOG("server started on %s:%d, with %d io and %d work threads.\n",host.c_str(),(int)port,(int)_ios,(int)_works);
	}
/*
	//work threads
	std::thread t(io_svc);
	t.join();
	std::mutex m;
	std::recursive_mutex rm;
	std::timed_mutex tm;
	std::recursive_timed_mutex rtm;

	std::unique_lock<std::mutex> lock(m);
	std::condition_variable cond;
	cond.wait(lock);
	cond.notify_one();
*/
}

void  Service_impl::_accept_one(){
	// Get new handler for accept,and bind with acceptor's io_service.
	sh_sptr sh(new Svc_handler_impl(_w,_a,_ios_io,_ios_work,_rb_size));
	// Use new handler to accept.
	_acceptor.async_accept(sh->_socket().lowest_layer(),
		boost::bind(&Service_impl::handle_accept,
			this,
			placeholders::error,
			sh));
}

/// Handle completion of an asynchronous accept operation.
void Service_impl::handle_accept(const boost::system::error_code& e,sh_sptr& sh){
	if(!e){
		// Start the first operation of the current handler.
		sh->_open();
		// Accept new connection.
		_accept_one();
	}else
		sh->_close(e);
}

void Service_impl::connect(const char* address,unsigned short port,unsigned short conns){
	if(!address||!port)return;

    // Connect with the internal endpoint.
	ip::tcp::endpoint ep(ip::address::from_string(address),port);
	for(unsigned short i=0;i<conns;++i){
		// Use new handler to connect.
		sh_sptr sh(new Svc_handler_impl(_w,_a,_ios_io,_ios_work,_rb_size));
		sh->_connect(ep);
	}
	if(!_started){
		_started=true;
		_ios_io.run();
	}
/*
	if(!service_)
		service_.reset(new service_type(&_w,_a,_ios,_works,_rb_size));
	service_->connect(address,port,conns);
//		size_t n=1;
//		while(!_started)
//			boost::this_thread::sleep(boost::posix_time::minutes(2));
*/
}
void Service_impl::set_timer(size_t id,size_t milliseconds){
	if(_timer_handler)_timer_handler->set_timer(id,milliseconds);
}
void Service_impl::unset_timer(size_t id){
	if(_timer_handler)_timer_handler->unset_timer(id);
}
void Service_impl::post_event(void* buf,size_t length){
//	if(service_)service_->post_event(buf,length);
}

void Service_impl::close(){
	if(_started){
		_started=false;
		_ios_io.stop();
	}
}
bool Service_impl::closed()const{
	return !_started;
}
// --------------------------------------------------------
static void sig_handler(const int sig)
{
    fprintf(stderr, "\nSIGINT handled, core terminated\n");
    exit(0);
}
/**
 * Set signal handler
 */
void _set_sig(){
	/*
    struct sigaction *sa = (struct sigaction *) malloc(sizeof(struct sigaction));

    // SIGINT
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGKILL, sig_handler);

    // Ignore SIGPIPE & SIGCLD
    sa->sa_handler = SIG_IGN;
    sa->sa_flags = 0;
    signal(SIGPIPE, SIG_IGN);
    if (sigemptyset(&sa->sa_mask) == -1 || sigaction(SIGPIPE, sa, 0) == -1)
    {
        fprintf(stderr, "Ignore SIGPIPE failed\n");
        exit(RTN_ERROR_SIGNAL);
    }
	*/
}
// --------------------------------------------------------
// service
// --------------------------------------------------------
Service::Service(Work_handler& w,io_alloc& a,size_t ios,size_t works,size_t rb_size){
	_svc.reset(new Service_impl(w,a,ios,works,rb_size));}
void Service::connect(const char* address,unsigned short port,unsigned short conns){if(_svc)_svc->connect(address,port,conns);}
void Service::run(unsigned short port,const char* address)	{if(_svc)_svc->run(port,address);}
void Service::close()										{if(_svc)_svc->close();}
bool Service::closed()const									{return _svc?_svc->closed():true;}
void Service::set_timer(size_t id,size_t milliseconds)		{if(_svc)_svc->set_timer(id,milliseconds);}
void Service::unset_timer(size_t id)						{if(_svc)_svc->unset_timer(id);}
void Service::post_event(void* buf,size_t length)			{if(_svc)_svc->post_event(buf,length);}
