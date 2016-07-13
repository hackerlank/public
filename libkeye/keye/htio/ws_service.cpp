// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_service.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"
// --------------------------------------------------------
typedef std::allocator<char> alloc_type;	//we can override this allocator
typedef websocketpp::server<websocketpp::config::asio> server_type;

#include "ws_handler.hpp"
//#include "work_handler.hpp"
namespace keye{
class ws_service_impl{
public:
	ws_service_impl(ws_service& w,size_t ios,size_t works,size_t rb_size)
	:/*_w(w),_ios(ios),_works(works),_rb_size(rb_size),*/_bExit(true){}

	void	run(unsigned short port,const char* address=nullptr){
		if (_bExit) {
			_bExit = false;

			try {
				// Set logging settings
				_server.set_error_channels(websocketpp::log::elevel::all);
				_server.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

				// Initialize Asio
				_server.init_asio();
				_server.set_reuse_addr(true);

				// Register our message handler
				_server.set_message_handler	(std::bind(&ws_service_impl::on_message,this, &_server, std::placeholders::_1, std::placeholders::_2));
				_server.set_http_handler	(std::bind(&ws_service_impl::on_http,	this, &_server, std::placeholders::_1));
				_server.set_fail_handler	(std::bind(&ws_service_impl::on_fail,	this, &_server, std::placeholders::_1)); 
				_server.set_open_handler	(std::bind(&ws_service_impl::on_open,	this, std::placeholders::_1));
				_server.set_close_handler	(std::bind(&ws_service_impl::on_close,	this, std::placeholders::_1));
				_server.set_validate_handler(std::bind(&ws_service_impl::validate,	this, &_server, std::placeholders::_1));
			}
			catch (const std::exception & e) {
				std::cout << e.what() << std::endl;
			}
			catch (websocketpp::lib::error_code e) {
				std::cout << e.message() << std::endl;
			}
			catch (...) {
				std::cout << "other exception" << std::endl;
			}

			// Listen on port
			_server.listen(port);
			// Queues a connection accept operation
			_server.start_accept();
			// Start the Asio io_service run loop in different thread
			_thread.reset(new std::thread(boost::bind(&server_type::run, &_server)));
		}
	}
	void	close(){
		if(!_bExit){
			/*
			if(ws_service_)
				ws_service_->stop();
			if(_thread){
				_thread->join();
				_thread.reset();
			}
			*/
			_bExit=true;
		}
	}
	bool	closed()const{
		return _bExit;
	}
	void	connect(const char* address,unsigned short port,unsigned short conns=1){
		_bExit=false;
		/*
		if(!ws_service_)
			ws_service_.reset(new ws_service_type(&_w,&_a,_ios,_works,_rb_size));
		ws_service_->connect(address,port,conns);
		*/
	}
	void	set_timer(size_t id,size_t milliseconds){
		//if(ws_service_)ws_service_->set_timer(id,milliseconds);
	}
	void	unset_timer(size_t id){
		//if(ws_service_)ws_service_->unset_timer(id);
	}
	void	post_event(void* buf,size_t length){
		//if(ws_service_)ws_service_->post_event(buf,length);
	}
private:

	// pull out the type of messages sent by our config
	typedef server_type::message_ptr message_ptr;

	bool validate(server_type *, websocketpp::connection_hdl) {
		//sleep(6);
		return true;
	}

	void on_http(server_type* s, websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = s->get_con_from_hdl(hdl);

		std::string res = con->get_request_body();

		std::stringstream ss;
		ss << "got HTTP request with " << res.size() << " bytes of body data.";

		con->set_body(ss.str());
		con->set_status(websocketpp::http::status_code::ok);
	}

	void on_fail(server_type* s, websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = s->get_con_from_hdl(hdl);

		std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message() << std::endl;
	}

	void on_open(websocketpp::connection_hdl) {
		std::cout << "Open handler" << std::endl;
	}

	void on_close(websocketpp::connection_hdl) {
		std::cout << "Close handler" << std::endl;
	}

	// Define a callback to handle incoming messages
	void on_message(server_type* s, websocketpp::connection_hdl hdl, message_ptr msg) {
		std::cout << "on_message called with hdl: " << hdl.lock().get()
			<< " and message: " << msg->get_payload()
			<< std::endl;

		try {
			s->send(hdl, msg->get_payload(), msg->get_opcode());
		}
		catch (const websocketpp::lib::error_code& e) {
			std::cout << "Echo failed because: " << e
				<< "(" << e.message() << ")" << std::endl;
		}
	}

	/*
	typedef bas::ws_service<work_handler_impl,alloc_type> ws_service_type;
	work_handler_impl	_w;
	alloc_type			_a;
	size_t				_ios,_works,_rb_size;
	std::shared_ptr<ws_service_type>	ws_service_;
	*/
	server_type						_server;

	std::shared_ptr<std::thread>	_thread;
	bool							_bExit;
};

// --------------------------------------------------------
// ws_service
// --------------------------------------------------------
ws_service::ws_service(size_t ios,size_t works,size_t rb_size){
	_svc.reset(new ws_service_impl(*this,ios,works,rb_size));
}

void ws_service::connect(const char* address,unsigned short port,unsigned short conns){
	if(_svc)_svc->connect(address,port,conns);
}
void ws_service::run(unsigned short port,const char* address){
	if(_svc)_svc->run(port,address);
}
void ws_service::close(){
	if(_svc)_svc->close();
}
bool ws_service::closed()const{
	return _svc?_svc->closed():true;
}
void ws_service::set_timer(size_t id,size_t milliseconds){
	if(_svc)_svc->set_timer(id,milliseconds);
}

void ws_service::unset_timer(size_t id){
	if(_svc)_svc->unset_timer(id);
}

void ws_service::post_event(void* buf,size_t length){
	if(_svc)_svc->post_event(buf,length);
}
};
