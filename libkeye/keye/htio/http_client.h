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
    class http_client_impl;
    class KEYE_API http_parser{
    public:
                            http_parser(bool request);
        void                set_uri(const char*);
        const char*         uri()const;

        void                set_version(const char*);
        const char*         version()const;
        
        void                set_method(const char*);
        const char*         method()const;
        
        void                set_header(const char*,const char*);
        void                set_headers(const char*);
        const char*         header(const char*)const;
        
        void                set_body(const char*);
        const char*         body()const;
        
        void                set_status(int,const char* =nullptr);
        int                 status(char** =nullptr);
        const std::string   raw()const;
    private:
        friend class http_client_impl;
        friend class ws_service_impl;
        std::shared_ptr<http_parser_impl>	_parser;
    };
    // --------------------------------------------------------
    // http client
    // --------------------------------------------------------
    class KEYE_API http_client{
    public:
                        http_client();
        void            request(const http_parser&);
        virtual void	on_response(const http_parser&){}

        //null svc_handler
        std::shared_ptr<svc_handler> shnull;
    private:
        std::shared_ptr<http_client_impl>	_svc;
    };
    // --------------------------------------------------------
};
#endif // _http_client_h_