// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: http_client.cpp
 *Desc		:
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-8-30
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"
// --------------------------------------------------------
typedef websocketpp::client<websocketpp::config::asio_client>	client_type;
typedef client_type service_type;

#include "ws_handler.hpp"
namespace keye{
    class http_client_impl{
    public:
        http_client_impl(http_client& w)
        :_handler(w){}
        
        void	close(){
            _client.stop();
        }
        void	connect(const char* address,unsigned short port,unsigned short conns=1){
            try {
                // Set logging settings
                _client.set_error_channels(websocketpp::log::elevel::all);
                _client.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
                
                // Initialize Asio
                _client.init_asio();
                _client.set_reuse_addr(true);
                _client.set_http_handler	(std::bind(&http_client_impl::on_http, this, std::placeholders::_1));
                _client.set_fail_handler	(std::bind(&http_client_impl::on_fail, this, std::placeholders::_1));
                _client.set_open_handler	(std::bind(&http_client_impl::on_open, this, std::placeholders::_1));
                _client.set_close_handler	(std::bind(&http_client_impl::on_close, this, std::placeholders::_1));
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
            
            char uri[128];
            sprintf(uri,"ws://%s:%d",address,port);
            websocketpp::lib::error_code ec;
            for(unsigned short i=0; i<conns;++i){
                auto con=_client.get_connection(uri,ec);
                if(ec)
                    std::cout<<"could not create connection because: "<<ec.message()<<std::endl;
                // Note that connect here only requests a connection. No network messages are
                // exchanged until the event loop starts running in the next line.
                else
                    _client.connect(con);
            }
            //_thread.reset(new std::thread(boost::bind(&client_type::run, &_client)));
        }
    private:
        // pull out the type of messages sent by our config
        typedef client_type::message_ptr message_ptr;
        
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
        http_client&					_handler;
    };
    
    // --------------------------------------------------------
    // http_client
    // --------------------------------------------------------
    http_client::http_client(){
        _svc.reset(new http_client_impl(*this));
    }
    
    void http_client::connect(const char* address,unsigned short port,unsigned short conns){
        if(_svc)_svc->connect(address,port,conns);
    }
    void http_client::close(){
        if(_svc)_svc->close();
    }
};
