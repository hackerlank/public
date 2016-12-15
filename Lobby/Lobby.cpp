// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LobbyFwd.h"

#ifdef WIN32
#include <conio.h>
#else
#include <unistd.h>
#endif

using namespace keye;

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

std::shared_ptr<keye::logger> sLogger;
std::shared_ptr<keye::logger> sDebug;

Lobby* Lobby::sLobby=nullptr;

Lobby::Lobby()
:Server("lobby"){
    sLobby=this;
}

bool Lobby::run(const char* cfg){
    if(Server::run(cfg)){
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
        return true;
    }
    return false;
}

void Lobby::on_http(const http_parser& req,const std::function<void(const http_parser&)> func){
    handler.on_http(req,func);
}

int main(int argc, char* argv[]) {
    const char* cfg="lobby.cfg";
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3&&arg[0]=='-')switch(arg[1]){
            case 'f':{
                cfg=&arg[2];
                break;
            }
            default:
                break;
        }
    }

    Lobby server;
	server.run(cfg);
    while(true)usleep(1000);

	return 0;
}
