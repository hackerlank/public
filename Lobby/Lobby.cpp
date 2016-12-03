// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LobbyFwd.h"

#ifdef WIN32
#include <conio.h>
#endif

using namespace keye;

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

Lobby* Lobby::sLobby=nullptr;

Lobby::Lobby(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size) {
    sLobby=this;
    
    // e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
    // standalone mode if only one node, else cluster mode.
    const char* dbhost="ec2-52-221-242-102.ap-southeast-1.compute.amazonaws.com:6379";
    spdb=std::make_shared<redis_proxy>();
    if(!spdb->connect(dbhost))
        spdb.reset();
    else{
        //init global id if necessary
        char idkey[32],idval[32];
        long long baseA=1000000000; //1,000,000,000
        long long baseB=100100;
        for(int i=1;i<=4;++i){
            auto id=baseA*i+baseB;
            sprintf(idkey,"global_id:%d",i);
            sprintf(idval,"%lld",id);
            spdb->setnx(idkey,idval);
        }
    }
}

void Lobby::on_http(const http_parser& req,http_parser& resp){
    handler.on_http(req,resp);
}

int main(int argc, char* argv[]) {
    unsigned short port = 8800;
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

	redis_proxy redis;
	keye::PacketWrapper pw;
	PBHelper helper(pw);

    Lobby server;
	server.run(port,"127.0.0.1");
	KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
