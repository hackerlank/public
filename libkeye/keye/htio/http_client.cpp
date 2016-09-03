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

    // --------------------------------------------------------
    // http parser
    // --------------------------------------------------------
    class http_parser_impl{
    public:
        http_parser_impl(bool request):_requests(request){
            if(request)
                _parser.reset(new websocketpp::http::parser::request);
            else
                _parser.reset(new websocketpp::http::parser::response);
        }
        void        set_uri(const char* s){
            if(s&&_requests)if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser))req->set_uri(s);
        }
        const char* uri()const{
            if(_requests)
                if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser))return req->get_uri().c_str();
            return nullptr;
        }
        void        set_version(const char* s){if(s)_parser->set_version(s);}
        const char* version()const{return _parser->get_version().c_str();}
        void        set_method(const char* s){
            if(_requests&&s)if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser))try{
                req->set_method(s);
            }catch (const std::exception & e) {
                KEYE_LOG("%s\n",e.what());
            }catch (...) {
                KEYE_LOG("%s\n","other exception");
            }
        }
        const char* method()const{
            if(_requests)
                if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser))return req->get_method().c_str();
            return nullptr;
        }
        void        set_header(const char* key,const char* val){
            if(key)try{
                if(val)
                    _parser->append_header(key,val);
                else
                    _parser->remove_header(key);
            }catch (const std::exception & e) {
                KEYE_LOG("%s\n",e.what());
            }catch (...) {
                KEYE_LOG("%s\n","other exception");
            }
        }
        void        set_headers(const char* raw){
            if(raw)try{
                if(_requests){
                    if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser)){
                        req->consume(raw,strlen(raw));
                    }
                }else if(auto req=std::static_pointer_cast<websocketpp::http::parser::response>(_parser)){
                    req->consume(raw,strlen(raw));
                }
            }catch (const std::exception & e) {
                KEYE_LOG("%s\n",e.what());
            }catch (...) {
                KEYE_LOG("%s\n","other exception");
            }
        }
        const char* header(const char* key)const{
            return key?_parser->get_header(key).c_str():nullptr;
        }
        void        set_body(const char* s){if(s)_parser->set_body(s);}
        const char* body()const{return _parser->get_body().c_str();}
        
        const std::string raw()const{
            if(_requests){
                if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(_parser))return req->raw();
            }else if(auto req=std::static_pointer_cast<websocketpp::http::parser::response>(_parser))return req->raw();
            return "";
        }
        void        set_status(int code,const char* msg){
            if(!_requests)
                if(auto req=std::static_pointer_cast<websocketpp::http::parser::response>(_parser)){
                    auto st=(websocketpp::http::status_code::value)code;
                    if(msg)
                        req->set_status(st,msg);
                    else
                        req->set_status(st);
                }
        }
        int         status(char** ppmsg){
            if(!_requests)
                if(auto req=std::static_pointer_cast<websocketpp::http::parser::response>(_parser)){
                    if(ppmsg)strcpy(*ppmsg,req->get_status_msg().c_str());
                    return req->get_status_code();
                }
            return websocketpp::http::status_code::ok;
        }
    private:
        friend class http_client_impl;
        friend class ws_service_impl;
        bool    _requests;
        std::shared_ptr<websocketpp::http::parser::parser>  _parser;
    };
    // --------------------------------------------------------
    // connection for http
    // --------------------------------------------------------
    class http_connect: public connection<config>{
        typedef typename config::transport_type transport_type;
        typedef typename transport_type::transport_con_type transport_con_type;
        typedef http_connect type;
        typedef lib::function<void(lib::error_code const &,const char*,size_t)> response_handler;
    public:
        typedef lib::shared_ptr<type> ptr;

        explicit http_connect(bool p_is_server, std::string const & ua, alog_type& alog,
                              elog_type& elog, rng_type & rng)
        :m_alog(alog),m_elog(elog),connection(p_is_server,ua,alog,elog,rng){}

        ptr get_shared() {
            return lib::static_pointer_cast<type>(transport_con_type::get_shared());
        }

        void async_send(const char* buf,size_t sz,response_handler h){
            transport_con_type::async_write(buf,sz,
                                            lib::bind(
                                                      &type::handle_send_request,
                                                      type::get_shared(),
                                                      lib::placeholders::_1,
                                                      h
                                                      )
                                            );
        }
        void handle_send_request(lib::error_code const & ec,response_handler h) {
            m_alog.write(log::alevel::devel,"handle_send_request");
            if (ec) {
                std::stringstream s;
                s << "handle_send_request error: " << ec << " (" << ec.message() << ")";
                m_elog.write(log::elevel::rerror, s.str());
                this->terminate(ec);
                return;
            }
            
            transport_con_type::async_read_at_least(
                                                    1,
                                                    m_buf,
                                                    config::connection_read_buffer_size,
                                                    lib::bind(
                                                              &type::handle_response,
                                                              type::get_shared(),
                                                              lib::placeholders::_1,
                                                              lib::placeholders::_2,
                                                              h
                                                              )
                                                    );
        }
        void handle_response(lib::error_code const & ec,size_t bytes_transferred,response_handler h){
            m_alog.write(log::alevel::devel,"handle_response");
            if(h)
                h(ec,m_buf,bytes_transferred);
        }

    private:
        char        m_buf[config::connection_read_buffer_size];
        alog_type&  m_alog;
        elog_type&  m_elog;
    };
    
    // --------------------------------------------------------
    // implements client for http
    // --------------------------------------------------------
    class http_client_impl: public endpoint<http_connect,config> {
        typedef http_client_impl client_type;
        typedef http_client_impl type;
        typedef lib::shared_ptr<type> ptr;
        typedef typename config::transport_type transport_type;
        typedef http_connect connection_type;
        typedef typename connection_type::ptr connection_ptr;
        typedef typename transport_type::transport_con_type transport_con_type;
        typedef endpoint<connection_type,config> endpoint_type;
    public:
        
        http_client_impl(http_client& w):_handler(w),endpoint_type(false){}

        ~http_client_impl(){
            stop();
            if(_thread){
                _thread->join();
                _thread.reset();
            }
        }
        
        void	request(const http_parser& parser){
            if(!_thread)try{
                //set_error_channels(websocketpp::log::elevel::all);
                //set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
                set_error_channels(websocketpp::log::elevel::none);
                set_access_channels(websocketpp::log::alevel::none);
                
                // Initialize Asio
                init_asio();
                set_reuse_addr(true);
            }catch (const std::exception & e) {
                KEYE_LOG("%s\n",e.what());
            }catch (websocketpp::lib::error_code e) {
                KEYE_LOG("%s\n",e.message().c_str());
            }catch (...) {
                KEYE_LOG("%s\n","other exception");
            }
            auto uri=parser.uri();
            websocketpp::lib::error_code ec;
            auto con=get_connection(uri,ec);
            if(ec){
                endpoint_type::m_elog.write(log::elevel::rerror,"request error: wrong uri");
            }else{
                transport_type::async_connect(
                                              lib::static_pointer_cast<transport_con_type>(con),
                                              con->get_uri(),
                                              lib::bind(
                                                        &type::handle_connect,
                                                        this,
                                                        con,
                                                        parser,
                                                        lib::placeholders::_1
                                                        )
                                              );
            }
            if(!_thread)
                _thread.reset(new std::thread(boost::bind(&type::run,this)));
        }

    private:
        void handle_connect(connection_ptr con,const http_parser& parser,lib::error_code const & ec) {
            if (ec) {
                con->terminate(ec);
                endpoint_type::m_elog.write(log::elevel::rerror,"handle_connect error: "+ec.message());
            } else {
                endpoint_type::m_alog.write(log::alevel::http,"Successful connection");
                
                if(parser._parser->_requests){
                    if(auto req=std::static_pointer_cast<websocketpp::http::parser::request>(parser._parser->_parser)){
                        auto uri=con->get_uri();
                        req->set_uri(uri->get_resource());
                        req->replace_header("Host",uri->get_host_port());
                        
                        auto req_buf = parser._parser->raw();
                        con->async_send(req_buf.data(),req_buf.size(),
                                        lib::bind(
                                                  &type::handle_response,
                                                  this,
                                                  lib::placeholders::_1,
                                                  lib::placeholders::_2,
                                                  lib::placeholders::_3
                                                  ));
                    }
                }else
                    endpoint_type::m_elog.write(log::alevel::http,"Request invalid");
            }
        }
        
        void handle_response(lib::error_code const & ec,const char* buf,size_t bytes_transferred){
            std::stringstream s;
            s<<"Http response "<<bytes_transferred<<" bytes";
            endpoint_type::m_alog.write(log::alevel::http,s.str());
            
            http_parser parser(false);
            if(auto req=std::static_pointer_cast<websocketpp::http::parser::response>(parser._parser->_parser)){
                try{
                    req->consume(buf,bytes_transferred);
                }catch (const std::exception & e) {
                    KEYE_LOG("%s\n",e.what());
                }catch (...) {
                    KEYE_LOG("%s\n","other exception");
                }
                _handler.on_response(parser);
            }else
                endpoint_type::m_elog.write(log::alevel::http,"Response invalid");
        }
        
        connection_ptr get_connection(std::string const & u, lib::error_code & ec) {
            uri_ptr location = lib::make_shared<uri>(u);
            if (!location->get_valid()) {
                ec = error::make_error_code(error::invalid_uri);
                return connection_ptr();
            }
            
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
        
        http_client&					_handler;
        std::shared_ptr<std::thread>	_thread;
    };
    
    // --------------------------------------------------------
    // null svc_handler
    // --------------------------------------------------------
    class null_svc_handler:public svc_handler{
    public:
        virtual void	close(){
        }
        virtual size_t	id()const{return 0;}
        virtual std::shared_ptr<svc_handler> operator()()const{
            return std::shared_ptr<svc_handler>(new null_svc_handler());
        }
        virtual void	send(void* buf,size_t length){
        }
        virtual void	post_event(void* buf,size_t length){
        }
        virtual void	set_timer(size_t id,size_t milliseconds){
        }
        virtual void	unset_timer(size_t id){
        }
        virtual std::string	address()const{
            return "0.0.0.0";
        }
        virtual unsigned short	port()const{
            return 0;
        }
        virtual std::shared_ptr<void>&	sptr(){
            return s;
        }
        void on_timer(size_t id, size_t milliseconds, websocketpp::lib::error_code const & ec) {
        }
    private:
        std::shared_ptr<void> s;
    };

    // --------------------------------------------------------
    // http_client
    // --------------------------------------------------------
    http_client::http_client(){
        _svc.reset(new http_client_impl(*this));
        shnull.reset(new null_svc_handler());
    }
    void http_client::request(const http_parser& parser){
        _svc->request(parser);
    }

    // --------------------------------------------------------
    // parser for http
    // --------------------------------------------------------
    http_parser::http_parser(bool request){
        _parser.reset(new http_parser_impl(request));
    }
    const char* http_parser::uri()const{
        return _parser->uri();
    }
    void http_parser::set_uri(const char* uri){
        _parser->set_uri(uri);
    }
    void http_parser::set_version(const char* ver){
        _parser->set_version(ver);
    }
    const char* http_parser::version()const{
        return _parser->version();
    }
    void http_parser::set_method(const char* m){
        _parser->set_method(m);
    }
    const char* http_parser::method()const{
        return _parser->method();
    }
    void http_parser::set_header(const char* key,const char* value){
        _parser->set_header(key,value);
    }
    void http_parser::set_headers(const char* raw){
        _parser->set_headers(raw);
    }
    const char* http_parser::header(const char* key)const{
        return _parser->header(key);
    }
    void http_parser::set_body(const char* body){
        _parser->set_body(body);
    }
    const char* http_parser::body()const{
        return _parser->body();
    }
    void http_parser::set_status(int code,const char* msg){
        _parser->set_status(code,msg);
    }
    int http_parser::status(char** ppmsg){
        return _parser->status(ppmsg);
    }
    const std::string http_parser::raw()const{
        return _parser->raw();
    }
};
