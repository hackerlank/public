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
typedef client_type service_type;

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
	void	connect(const char* uri,unsigned short conns=1){
		_bExit=false;

		try {
			// Set logging settings
            //_client.set_error_channels(websocketpp::log::elevel::all);
            //_client.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
			_client.set_error_channels(websocketpp::log::elevel::none);
			_client.set_access_channels(websocketpp::log::alevel::none);

			// Initialize Asio
			_client.init_asio();
			_client.set_reuse_addr(true);
			_timer_con=_client.get_connection();

			// Register our message handler
			_client.set_message_handler	(std::bind(&ws_client_impl::on_message, this, std::placeholders::_1, std::placeholders::_2));
			_client.set_fail_handler	(std::bind(&ws_client_impl::on_fail, this, std::placeholders::_1));
			_client.set_open_handler	(std::bind(&ws_client_impl::on_open, this, std::placeholders::_1));
			_client.set_close_handler	(std::bind(&ws_client_impl::on_close, this, std::placeholders::_1));
			_client.set_validate_handler(std::bind(&ws_client_impl::validate, this, std::placeholders::_1));
		}catch (const std::exception & e) {
            KEYE_LOG("%s\n",e.what());
		}catch (websocketpp::lib::error_code e) {
            KEYE_LOG("%s\n",e.message().c_str());
		}catch (...) {
            KEYE_LOG("%s\n","other exception");
		}

		websocketpp::lib::error_code ec;
		for(unsigned short i=0; i<conns;++i){
			auto con=_client.get_connection(uri,ec);
			if(ec)
                KEYE_LOG("could not create connection because: %s\n",ec.message().c_str());
			// Note that connect here only requests a connection. No network messages are
			// exchanged until the event loop starts running in the next line.
			else
				_client.connect(con);
		}
		_thread.reset(new std::thread(boost::bind(&client_type::run, &_client)));
	}
	void	set_timer(size_t id, size_t milliseconds) {
		unset_timer(id);
		timers_[id] = _client.set_timer((long)milliseconds, std::bind(&ws_client_impl::on_timer, this, id, milliseconds, std::placeholders::_1));
	}
	void	unset_timer(size_t id) {
		auto i = timers_.find(id);
		if (i != timers_.end())
			i->second->cancel();
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
	}

	void on_close(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
	}

	void on_fail(websocketpp::connection_hdl hdl) {
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_close(sh);
        KEYE_LOG("on_fail: %d %s\n",con->get_ec().value(),con->get_ec().message().c_str());
	}

	// Define a callback to handle incoming messages
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		auto& pl = msg->get_payload();
		client_type::connection_ptr con = _client.get_con_from_hdl(hdl);
		ws_handler_impl sh(con);
		_handler.on_read(sh,(void*)pl.data(),pl.length());
	}

	void on_timer(size_t id, size_t milliseconds, websocketpp::lib::error_code const & ec) {
		ws_handler_impl sh(_timer_con);
		_handler.on_timer(sh,id, milliseconds);
		// set timer for next telemetry check
		if (!ec)set_timer(id, milliseconds);
	}

	client_type						_client;
	ws_client&						_handler;
	client_type::connection_ptr		_timer_con;	//connection for timer
	// Events timers
	std::map<std::size_t, client_type::timer_ptr> timers_;

	std::shared_ptr<std::thread>	_thread;
	bool							_bExit;
};

// --------------------------------------------------------
// ws_client
// --------------------------------------------------------
ws_client::ws_client(size_t ios,size_t works,size_t rb_size){
	_svc.reset(new ws_client_impl(*this,ios,works,rb_size));
}

void ws_client::connect(const char* uri,unsigned short conns){
	if(_svc)_svc->connect(uri,conns);
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
