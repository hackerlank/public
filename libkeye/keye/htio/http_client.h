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
    /* http_client to process io,
     io runs on io threads,events and timer run on work threads,
     threads are auto chosen */
    // --------------------------------------------------------
    class http_client_impl;
    
    class KEYE_API http_client{
    public:
        //ios:io threads;works:work threads;rb_size:read buffer max size
                http_client();
        virtual	~http_client(){close();}
        //run as client and connect to server
        void	connect(const char* address,unsigned short port,unsigned short conns=1);
        void	close();
        
        //events handlers
        virtual void	on_open(svc_handler&){}
        virtual void	on_close(svc_handler&){}
        virtual void	on_read(svc_handler&,void*,size_t){}
        virtual void	on_write(svc_handler&,void*,size_t){}
    private:
        std::shared_ptr<http_client_impl>	_svc;
    };
    // --------------------------------------------------------
};
#endif // _http_client_h_