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

bool Lobby::on_timer(svc_handler&, size_t id, size_t milliseconds) {
    switch (id) {
        case TIMER::TIMER_SEC:
            break;
        case TIMER::TIMER_MIN:
            break;
        case TIMER::TIMER_HOUR:{
            time_t t=time(NULL);
            tm* aTm=localtime(&t);
            if(aTm->tm_hour==0)
                setup_log(name.c_str());
            
            decltype(t) thresh=60*60;  //1 hour
            for(auto i=sessions.begin();i!=sessions.end();){
                if(i->second-t>thresh){
                    i=sessions.erase(i);
                }else
                    ++i;
            }
            break;
        }
        case TIMER::TIMER_DAY:{
            break;
        }
        default:
            break;
    }
    return true;
}

int main(int argc, char* argv[]) {
    /*
    std::string contents[]={
    "CKMfEgo0MDAwMTAwMTAzINmQ%2ffSRsMCbCQ%3d%3d",
    "CKMfEgo0MDAwMTAwMTAzINmi%2bv%2b7q8DAmQE%3d",
    "CKMfEgo0MDAwMTAwMTAzINnW%2fJWDt4DAWQ%3d%3d",
    "CKMfEgo0MDAwMTAwMTAzINmQo7nwsYCFxwE%3d",
    };
    
    for(int i=0;i<4;++i){
        auto& content=contents[i];
        printf("content=%s\n",content.c_str());
        auto str=UrlDecode(content);
        printf("  content=%s\n",str.c_str());
        str=base64_decode(content);
        printf("  decode=%s\n",str.c_str());
        proto3::MsgCLLobby imsg;
        if(imsg.ParseFromString(str)){
            printf("  OK: version=%d,session=%lld\n",imsg.version(),imsg.session());
        }else
            printf("--error\n");
    }
    return 0;
    */
    
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
    server.set_timer(TIMER::TIMER_HOUR, 1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);
    while(true)msleep(1000000);

	return 0;
}
