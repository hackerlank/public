// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Fwd.h"

#ifdef WIN32
#include <conio.h>
#endif

using namespace keye;

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

Server* Server::sServer=nullptr;

Server::Server(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size) {
    sServer=this;
}

void Server::on_http(const http_parser& req,http_parser& resp){
    handler.on_http(req,resp);
}

int main(int argc, char* argv[]) {
    unsigned short port = 8810;
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3&&arg[0]=='-')switch(arg[1]){
            case 'p':{
                auto a=&arg[2];
                try{
                    port=atoi(a);
                }catch(...){}
                break;
            }
            case 'w':
                break;
            case 'i':
            default:
                break;
        }
    }
	proto3::MsgSCLogin zi;

	redis_proxy redis;
	keye::PacketWrapper pw;
	PBHelper helper(pw);

    Server server;
	server.run(port,"127.0.0.1");
	KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
