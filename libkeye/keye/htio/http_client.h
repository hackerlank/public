// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: http_client.h
 *Desc		:
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-8-30
 */
// --------------------------------------------------------
#ifndef _http_client_h_
#define _http_client_h_

#pragma warning(disable:4251)	//avoid export implementation class

namespace keye{
    // --------------------------------------------------------
    // http parser
    // --------------------------------------------------------
    class http_parser_impl;
    class KEYE_API http_parser{
    public:
        http_parser();
        void                set_uri(const char*);
        void                set_version(const char*);
        void                set_method(const char*);
        void                set_header(const char*,const char*);
        void                set_body(const char*);
        const std::string   raw();

        const char*         version();
        int                 code();
        const char*         status();
        const char*         header(const char*);
        const char*         body();
    private:
        std::shared_ptr<http_parser_impl>	_parser;
    };
    // --------------------------------------------------------
    // http client
    // --------------------------------------------------------
    class http_client_impl;
    class KEYE_API http_client{
    public:
        //ios:io threads;works:work threads;rb_size:read buffer max size
                http_client();
        //run as client and connect to server
        void	request(const char* address,const char* content,unsigned short port=80);
        
        //events handlers
        virtual void	on_response(void*,size_t){}

        //null svc_handler
        std::shared_ptr<svc_handler> shnull;
    private:
        std::shared_ptr<http_client_impl>	_svc;
    };
    // --------------------------------------------------------
};
#endif // _http_client_h_