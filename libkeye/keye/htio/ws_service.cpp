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
typedef websocketpp::server<websocketpp::config::asio>			server_type;
typedef server_type service_type;

#include "ws_handler.hpp"
namespace keye{
class ws_service_impl{
public:
	ws_service_impl(ws_service& w,size_t ios,size_t works,size_t rb_size)
	:_handler(w),_bExit(true){}

	void	run(unsigned short port,const char* address=nullptr){
		if (_bExit) {
			_bExit = false;

			try {
				// Set logging settings
                _server.set_error_channels(websocketpp::log::elevel::none);
                _server.set_access_channels(websocketpp::log::alevel::none);
				//_server.set_error_channels(websocketpp::log::elevel::all);
				//_server.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

				// Initialize Asio
				_server.init_asio();
				_server.set_reuse_addr(true);
				_timer_con = _server.get_connection();

				// Register our message handler
				_server.set_message_handler	(std::bind(&ws_service_impl::on_message,this, std::placeholders::_1, std::placeholders::_2));
				_server.set_http_handler	(std::bind(&ws_service_impl::on_http,	this, std::placeholders::_1));
				_server.set_fail_handler	(std::bind(&ws_service_impl::on_fail,	this, std::placeholders::_1)); 
				_server.set_open_handler	(std::bind(&ws_service_impl::on_open,	this, std::placeholders::_1));
				_server.set_close_handler	(std::bind(&ws_service_impl::on_close,	this, std::placeholders::_1));
				_server.set_validate_handler(std::bind(&ws_service_impl::validate,	this, std::placeholders::_1));
			}catch (const std::exception & e) {
                KEYE_LOG("%s\n",e.what());
            }catch (websocketpp::lib::error_code e) {
                KEYE_LOG("%s\n",e.message().c_str());
            }catch (...) {
                KEYE_LOG("%s\n","other exception");
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
			_server.stop();
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
	}
	void	set_timer(size_t id,size_t milliseconds){
		unset_timer(id);
		timers_[id] = _server.set_timer((long)milliseconds, std::bind(&ws_service_impl::on_timer, this, id, milliseconds, std::placeholders::_1));
	}
	void	unset_timer(size_t id){
		auto i = timers_.find(id);
		if (i != timers_.end())
			i->second->cancel();
	}
	void	post_event(void* buf,size_t length){
		//if(ws_service_)ws_service_->post_event(buf,length);
	}
private:
	// pull out the type of messages sent by our config
	typedef server_type::message_ptr message_ptr;

	bool validate(websocketpp::connection_hdl) {
		//sleep(6);
		return true;
	}

	void on_open(websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = _server.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_open(sh);
	}

	void on_close(websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = _server.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
	}

	void on_fail(websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = _server.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
        //KEYE_LOG("on_fail: %d %s\n",con->get_ec().value(),con->get_ec().message().c_str());
	}

	// Define a callback to handle incoming messages
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		auto& pl = msg->get_payload();
		server_type::connection_ptr con = _server.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_read(sh,(void*)pl.data(),pl.length());
	}

	void on_http(websocketpp::connection_hdl hdl) {
		server_type::connection_ptr con = _server.get_con_from_hdl(hdl);
        http_parser req_parser(true),resp_parser(false);

        auto& req=con->get_request();
        req_parser.set_uri(req.get_uri().c_str());
        req_parser.set_version(req.get_version().c_str());
        req_parser.set_method(req.get_method().c_str());
        req_parser.set_headers(req.raw_head().c_str());
        req_parser.set_body(req.get_body().c_str());

        //handle and fill response
        _handler.on_http(req_parser,resp_parser);

        auto resp=(websocketpp::http::parser::response*)&con->get_response();
        resp->consume(resp_parser.raw().c_str(),resp_parser.raw().size());
        
        //will close connection after sending response
	}

	void on_timer(size_t id, size_t milliseconds, websocketpp::lib::error_code const & ec) {
		ws_handler_impl sh(_timer_con);
		_handler.on_timer(sh, id, milliseconds);
		// set timer for next telemetry check
		if (!ec)set_timer(id, milliseconds);
	}

	server_type						_server;
	ws_service&						_handler;
	server_type::connection_ptr		_timer_con;	//connection for timer
	// Events timers
	std::map<std::size_t, server_type::timer_ptr> timers_;

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
