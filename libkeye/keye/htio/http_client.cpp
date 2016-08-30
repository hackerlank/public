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
using namespace websocketpp;

namespace keye{
    typedef websocketpp::config::asio_client config;

    class http_connect: public connection<config>{
        typedef typename config::transport_type transport_type;
        typedef typename transport_type::transport_con_type transport_con_type;
    public:
        typedef http_connect type;
        typedef lib::shared_ptr<type> ptr;
        typedef lib::function<void(lib::error_code const &,const char*,size_t)> response_handler;

        explicit http_connect(bool p_is_server, std::string const & ua, alog_type& alog,
                              elog_type& elog, rng_type & rng)
        :connection(p_is_server,ua,alog,elog,rng){}

        ptr get_shared() {
            return lib::static_pointer_cast<type>(transport_con_type::get_shared());
        }

        void async_send(const char* buf,size_t sz,response_handler h){
            transport_con_type::async_write(buf,sz,
                                            lib::bind(
                                                      &type::handle_send_http_request,
                                                      type::get_shared(),
                                                      lib::placeholders::_1,
                                                      h
                                                      )
                                            );
        }

        void handle_send_http_request(lib::error_code const & ec,response_handler h) {
            //m_alog.write(log::alevel::devel,"handle_send_http_request");
            
            lib::error_code ecm = ec;
            if (ecm) {
                if (ecm == transport::error::eof/* && m_state == session::state::closed*/) {
                    // we expect to get eof if the connection is closed already
//                    m_alog.write(log::alevel::devel,"got (expected) eof/state error from closed con");
                    return;
                }
                
                //log_err(log::elevel::rerror,"handle_send_http_request",ecm);
                this->terminate(ecm);
                return;
            }
            
            transport_con_type::async_read_at_least(
                                                    1,
                                                    m_buf,
                                                    config::connection_read_buffer_size,
                                                    lib::bind(
                                                              &type::handle_http_response,
                                                              type::get_shared(),
                                                              lib::placeholders::_1,
                                                              lib::placeholders::_2,
                                                              h
                                                              )
                                                    );
        }
        void handle_http_response(lib::error_code const & ec,size_t bytes_transferred,response_handler h){
            if(h)
                h(ec,m_buf,bytes_transferred);
        }

        char                    m_buf[config::connection_read_buffer_size];
    };
    
    class http_client_impl: public endpoint<http_connect,config> {
        typedef http_client_impl client_type;
    public:
        typedef http_client_impl type;
        typedef lib::shared_ptr<type> ptr;
        
        typedef typename config::concurrency_type concurrency_type;
        typedef typename config::transport_type transport_type;
        
        typedef http_connect connection_type;
        typedef typename connection_type::ptr connection_ptr;
        
        typedef typename transport_type::transport_con_type transport_con_type;
        typedef typename transport_con_type::ptr transport_con_ptr;
        
        typedef endpoint<connection_type,config> endpoint_type;
        
        friend class connection<config>;

        http_client_impl(http_client& w)
        :_handler(w),endpoint_type(false)
        {
            endpoint_type::m_alog.write(log::alevel::devel, "client constructor");
            // Set logging settings

            set_error_channels(websocketpp::log::elevel::all);
            set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
            
            // Initialize Asio
            init_asio();
            set_reuse_addr(true);
            
            set_http_handler	(std::bind(&type::on_http, this, std::placeholders::_1));
            set_fail_handler	(std::bind(&type::on_fail, this, std::placeholders::_1));
            set_open_handler	(std::bind(&type::on_open, this, std::placeholders::_1));
            set_close_handler	(std::bind(&type::on_close,this, std::placeholders::_1));
        }
        
    private:
        connection_ptr get_connection(uri_ptr location, lib::error_code & ec) {
            if (location->get_secure() && !transport_type::is_secure()) {
                ec = error::make_error_code(error::endpoint_not_secure);
                return connection_ptr();
            }
            
            connection_ptr con = endpoint_type::create_connection();
            
            if (!con) {
                ec = error::make_error_code(error::con_creation_failed);
                return con;
            }
            
            con->set_uri(location);
            
            ec = lib::error_code();
            return con;
        }
        
        connection_ptr get_connection(std::string const & u, lib::error_code & ec) {
            uri_ptr location = lib::make_shared<uri>(u);
            
            if (!location->get_valid()) {
                ec = error::make_error_code(error::invalid_uri);
                return connection_ptr();
            }
            
            return get_connection(location, ec);
        }
        
        connection_ptr connect(connection_ptr con) {
            endpoint_type::m_alog.write(log::alevel::connect,"start connecting ...");
            // Ask transport to perform a connection
            transport_type::async_connect(
                                          lib::static_pointer_cast<transport_con_type>(con),
                                          con->get_uri(),
                                          lib::bind(
                                                    &type::handle_connect,
                                                    this,
                                                    con,
                                                    lib::placeholders::_1
                                                    )
                                          );
            
            return con;
        }
        // handle_connect
        void handle_connect(connection_ptr con, lib::error_code const & ec) {
            if (ec) {
                con->terminate(ec);
                endpoint_type::m_elog.write(log::elevel::rerror,
                                            "handle_connect error: "+ec.message());
            } else {
                endpoint_type::m_alog.write(log::alevel::connect,"Successful connection");
                //con->start();
                
                typedef typename config::request_type request_type;
                typedef typename config::response_type response_type;
                request_type            req;
                response_type           m_response;
                
                auto uri=con->get_uri();
                
                req.set_method("GET");
                req.set_uri(uri->get_resource());
                req.set_version("HTTP/1.1");
                
                //req.append_header("Upgrade","websocket");
                //req.append_header("Connection","Upgrade");
                //req.replace_header("Sec-WebSocket-Version","13");
                req.replace_header("Host",uri->get_host_port());

                auto m_handshake_buffer = req.raw();
//                con->send(m_handshake_buffer);
                endpoint_type::m_alog.write(log::alevel::connect,"request sent");
                
                con->async_send(m_handshake_buffer.data(),
                                                m_handshake_buffer.size(),
                                                lib::bind(
                                                          &type::handle_http_response,
                                                          this,
//                                                          type::get_shared(),
                                                          lib::placeholders::_1,
                                                          lib::placeholders::_2,
                                                          lib::placeholders::_3
                                                          )
                                                );
                

                /*
                if (con->m_internal_state != istate::USER_INIT) {
                    m_alog.write(log::alevel::devel,"Start called in invalid state");
                    this->terminate(error::make_error_code(error::invalid_state));
                    return;
                }
                
                m_internal_state = istate::TRANSPORT_INIT;
                
                // Depending on how the transport implements init this function may return
                // immediately and call handle_transport_init later or call
                // handle_transport_init from this function.
                transport_con_type::init(
                                         lib::bind(
                                                   &type::handle_transport_init,
                                                   type::get_shared(),
                                                   lib::placeholders::_1
                                                   )
                                         );
                */
            }
        }
        std::shared_ptr<std::thread>	_thread;

        
        void handle_http_response(lib::error_code const & ec,const char* buf,
                                       size_t bytes_transferred){
            std::cout<<"----handle http bytes "<<bytes_transferred<<std::endl;
        }
        
        void handle_read_http_response(lib::error_code const & ec,
                                                           size_t bytes_transferred)
        {
            m_alog.write(log::alevel::devel,"handle_read_http_response");
            
            lib::error_code ecm = ec;
            /*
            if (!ecm) {
                scoped_lock_type lock(m_connection_state_lock);
                
                if (m_state == session::state::connecting) {
                    if (m_internal_state != istate::READ_HTTP_RESPONSE) {
                        ecm = error::make_error_code(error::invalid_state);
                    }
                } else if (m_state == session::state::closed) {
                    // The connection was canceled while the response was being sent,
                    // usually by the handshake timer. This is basically expected
                    // (though hopefully rare) and there is nothing we can do so ignore.
                    m_alog.write(log::alevel::devel,
                                 "handle_read_http_response invoked after connection was closed");
                    return;
                } else {
                    ecm = error::make_error_code(error::invalid_state);
                }
            }
            
            if (ecm) {
                if (ecm == transport::error::eof && m_state == session::state::closed) {
                    // we expect to get eof if the connection is closed already
                    m_alog.write(log::alevel::devel,
                                 "got (expected) eof/state error from closed con");
                    return;
                }
                
                log_err(log::elevel::rerror,"handle_read_http_response",ecm);
                this->terminate(ecm);
                return;
            }
            
            size_t bytes_processed = 0;
            // TODO: refactor this to use error codes rather than exceptions
            try {
                bytes_processed = m_response.consume(m_buf,bytes_transferred);
            } catch (http::exception & e) {
                m_elog.write(log::elevel::rerror,
                             std::string("error in handle_read_http_response: ")+e.what());
                this->terminate(make_error_code(error::general));
                return;
            }
            
            m_alog.write(log::alevel::devel,std::string("Raw response: ")+m_response.raw());
            
            if (m_response.headers_ready()) {
                if (m_handshake_timer) {
                    m_handshake_timer->cancel();
                    m_handshake_timer.reset();
                }
                
                lib::error_code validate_ec = m_processor->validate_server_handshake_response(
                                                                                              m_request,
                                                                                              m_response
                                                                                              );
                if (validate_ec) {
                    log_err(log::elevel::rerror,"Server handshake response",validate_ec);
                    this->terminate(validate_ec);
                    return;
                }
                
                // Read extension parameters and set up values necessary for the end
                // user to complete extension negotiation.
                std::pair<lib::error_code,std::string> neg_results;
                neg_results = m_processor->negotiate_extensions(m_response);
                
                if (neg_results.first) {
                    // There was a fatal error in extension negotiation. For the moment
                    // kill all connections that fail extension negotiation.
                    
                    // TODO: deal with cases where the response is well formed but 
                    // doesn't match the options requested by the client. Its possible
                    // that the best behavior in this cases is to log and continue with
                    // an unextended connection.
                    m_alog.write(log::alevel::devel, "Extension negotiation failed: " 
                                 + neg_results.first.message());
                    this->terminate(make_error_code(error::extension_neg_failed));
                    // TODO: close connection with reason 1010 (and list extensions)
                }
                
                // response is valid, connection can now be assumed to be open      
                m_internal_state = istate::PROCESS_CONNECTION;
                m_state = session::state::open;
                
                this->log_open_result();
                
                if (m_open_handler) {
                    m_open_handler(m_connection_hdl);
                }
                
                // The remaining bytes in m_buf are frame data. Copy them to the
                // beginning of the buffer and note the length. They will be read after
                // the handshake completes and before more bytes are read.
                std::copy(m_buf+bytes_processed,m_buf+bytes_transferred,m_buf);
                m_buf_cursor = bytes_transferred-bytes_processed;
                
                this->handle_read_frame(lib::error_code(), m_buf_cursor);
            } else {
                transport_con_type::async_read_at_least(
                                                        1,
                                                        m_buf,
                                                        config::connection_read_buffer_size,
                                                        lib::bind(
                                                                  &type::handle_read_http_response,
                                                                  type::get_shared(),
                                                                  lib::placeholders::_1,
                                                                  lib::placeholders::_2
                                                                  )
                                                        );
            }
            */
        }

    public:
        void	close(){
            stop();
        }
        void	request(const char* address,const char* content,unsigned short port){
            char uri[128];
            sprintf(uri,"http://%s:%d",address,port);
            websocketpp::lib::error_code ec;
            auto con=get_connection(uri,ec);
            if(ec){
                endpoint_type::m_elog.write(log::elevel::rerror,"request error: wrong uri");
            }else{
                connect(con);
            }
            _thread.reset(new std::thread(boost::bind(&type::run,this)));
            /*
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
            */
        }
    private:
        // pull out the type of messages sent by our config
        typedef client_type::message_ptr message_ptr;
 
        void on_open(websocketpp::connection_hdl hdl) {
            client_type::connection_ptr con = get_con_from_hdl(hdl);
 //           _handler.on_open(sh);
            std::cout << "Open handler" << std::endl;
        }
        
        void on_close(websocketpp::connection_hdl hdl) {
//            _handler.on_close(sh);
            std::cout << "Close handler" << std::endl;
        }
        
        void on_fail(websocketpp::connection_hdl hdl) {
            client_type::connection_ptr con = get_con_from_hdl(hdl);
//            _handler.on_close(sh);
            std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message() << std::endl;
        }
        
        void on_http(websocketpp::connection_hdl hdl) {
            client_type::connection_ptr con = get_con_from_hdl(hdl);
            
            std::string res = con->get_request_body();
            
            std::stringstream ss;
            ss << "got HTTP request with " << res.size() << " bytes of body data.";
            std::cout << "on_http" << ss.str()<<std::endl;
        }

        //client_type						_client;
        http_client&					_handler;
    };
    
    // --------------------------------------------------------
    // http_client
    // --------------------------------------------------------
    http_client::http_client(){
        _svc.reset(new http_client_impl(*this));
    }
    
    void http_client::request(const char* address,const char* content,unsigned short port){
        if(_svc)_svc->request(address,content,port);
    }
    void http_client::close(){
        if(_svc)_svc->close();
    }
};
