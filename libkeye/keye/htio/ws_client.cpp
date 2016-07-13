// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ws_client.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"
// --------------------------------------------------------
typedef websocketpp::client<websocketpp::config::asio_client>	client_type;
typedef client_type server_type;

#include "ws_handler.hpp"
namespace keye{
class ws_client_impl{
public:
	ws_client_impl(ws_client& w,size_t ios,size_t works,size_t rb_size)
	:_handler(w),_bExit(true){}

	void	run(unsigned short port,const char* address=nullptr){
		if (_bExit) {
			_bExit = false;
		}
	}
	void	close(){
		if(!_bExit){
			_client.stop();
			if(_thread){
				_thread->join();
				_thread.reset();
			}
			_bExit=true;
		}
	}
	bool	closed()const{
		return _bExit;
	}
	void	connect(const char* address,unsigned short port,unsigned short conns=1){
		_bExit=false;

		try {
			// Set logging settings
			_client.set_error_channels(websocketpp::log::elevel::all);
			_client.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

			// Initialize Asio
			_client.init_asio();
			_client.set_reuse_addr(true);

			// Register our message handler
			_client.set_message_handler	(std::bind(&ws_client_impl::on_message, this, std::placeholders::_1, std::placeholders::_2));
			_client.set_http_handler	(std::bind(&ws_client_impl::on_http, this, std::placeholders::_1));
			_client.set_fail_handler	(std::bind(&ws_client_impl::on_fail, this, std::placeholders::_1));
			_client.set_open_handler	(std::bind(&ws_client_impl::on_open, this, std::placeholders::_1));
			_client.set_close_handler	(std::bind(&ws_client_impl::on_close, this, std::placeholders::_1));
			_client.set_validate_handler(std::bind(&ws_client_impl::validate, this, std::placeholders::_1));
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
	typedef client_type::message_ptr message_ptr;

	bool validate(websocketpp::connection_hdl) {
		//sleep(6);
		return true;
	}

	void on_open(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_open(sh);
		std::cout << "Open handler" << std::endl;
	}

	void on_close(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
		std::cout << "Close handler" << std::endl;
	}

	void on_fail(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
		std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message() << std::endl;
	}

	// Define a callback to handle incoming messages
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		auto& pl = msg->get_payload();
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_read(sh,(void*)pl.data(),pl.length());

		std::cout << "on_message called with hdl: " << hdl.lock().get()
			<< " and message: " << msg->get_payload()
			<< std::endl;
		/*
		try {
			_server.send(hdl, msg->get_payload(), msg->get_opcode());
		}
		catch (const websocketpp::lib::error_code& e) {
			std::cout << "Echo failed because: " << e
				<< "(" << e.message() << ")" << std::endl;
		}
		*/
	}

	void on_http(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);

		std::string res = con->get_request_body();

		std::stringstream ss;
		ss << "got HTTP request with " << res.size() << " bytes of body data.";
		/* response
		con->set_body(ss.str());
		con->set_status(websocketpp::http::status_code::ok);
		*/
	}

	client_type						_client;
	ws_client						_handler;

	std::shared_ptr<std::thread>	_thread;
	bool							_bExit;
};

// --------------------------------------------------------
// ws_client
// --------------------------------------------------------
ws_client::ws_client(size_t ios,size_t works,size_t rb_size){
	_svc.reset(new ws_client_impl(*this,ios,works,rb_size));
}

void ws_client::connect(const char* address,unsigned short port,unsigned short conns){
	if(_svc)_svc->connect(address,port,conns);
}
void ws_client::run(unsigned short port,const char* address){
	if(_svc)_svc->run(port,address);
}
void ws_client::close(){
	if(_svc)_svc->close();
}
bool ws_client::closed()const{
	return _svc?_svc->closed():true;
}
void ws_client::set_timer(size_t id,size_t milliseconds){
	if(_svc)_svc->set_timer(id,milliseconds);
}

void ws_client::unset_timer(size_t id){
	if(_svc)_svc->unset_timer(id);
}

void ws_client::post_event(void* buf,size_t length){
	if(_svc)_svc->post_event(buf,length);
}
};
