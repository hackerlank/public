// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NodeFwd.h"

#ifdef WIN32
#include <conio.h>
#else
#include <unistd.h>
#endif

using namespace keye;
using namespace proto3;

enum TIMER:size_t{
    TIMER_SEC=100,
    TIMER_MIN,
    TIMER_HOUR,
    TIMER_DAY,
};

int main(int argc, char* argv[]) {
    unsigned short port = 8820;
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

    Immortal server;
    server.registerRule(std::make_shared<DoudeZhu>());
    server.registerRule(std::make_shared<Mahjong>());
    server.registerRule(std::make_shared<Paohuzi>());

	server.run(port,"127.0.0.1");
    server.set_timer(TIMER::TIMER_SEC, 1000);
    server.set_timer(TIMER::TIMER_MIN, 1000*60);
    server.set_timer(TIMER::TIMER_HOUR,1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);

    //DoudeZhu::test();
    Logger<<"server start at "<<port<<endf;
    while(true)usleep(1000);

	return 0;
}

