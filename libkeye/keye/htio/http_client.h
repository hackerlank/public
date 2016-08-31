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
        http_parser();
        void                set_uri(const char*);
        void                set_version(const char*);
        void                set_method(const char*);
        void                set_header(const char*,const char*);
        void                set_body(const char*);
        const char*         uri()const;
        const char*         method()const;
        const std::string   raw()const;

        const char*         version()const;
        int                 code();
        const char*         status();
        const char*         header(const char*)const;
        const char*         body()const;
    private:
        friend class http_client_impl;
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