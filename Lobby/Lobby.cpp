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

Lobby::Lobby(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size) {
    sLobby=this;
    setup_log("lobby");
}

void Lobby::run(const char* cfg){
    keye::ini_cfg_file  config;
    if(cfg && config.load(cfg)){
        auto port=(short)(int)config.value("port");
        ws_service::run(port,"127.0.0.1");
        Debug<<"server start at "<<port<<endf;
        
        // e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
        // standalone mode if only one node, else cluster mode.
        char db[128];
        sprintf(db,"%s:%d",(const char*)config.value("dbhost"),(int)config.value("dbport"));
        spdb=std::make_shared<redis_proxy>();
        if(!spdb->connect(db))
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
    }else{
        Debug<<"server start error: no config file"<<endf;
    }
}

void Lobby::on_http(const http_parser& req,http_parser& resp){
    handler.on_http(req,resp);
}

void Lobby::setup_log(const char* file){
#if defined(WIN32) || defined(__APPLE__)
    sLogger=std::make_shared<logger>();
    sDebug=std::make_shared<logger>();
#else
    time_t t=time(NULL);
    tm* aTm=localtime(&t);
    char logfile[32];
    sprintf(logfile,"%s-%02d-%02d-%02d.log",file,aTm->tm_year%100,aTm->tm_mon+1,aTm->tm_mday);
    sLogger=std::make_shared<logger>(logfile);
    sprintf(logfile,"%sD-%02d-%02d-%02d.log",file,aTm->tm_year%100,aTm->tm_mon+1,aTm->tm_mday);
    sDebug=std::make_shared<logger>(logfile);
#endif
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
