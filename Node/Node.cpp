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
using namespace std;

std::shared_ptr<keye::logger> sLogger;
std::shared_ptr<keye::logger> sDebug;

int main(int argc, char* argv[]) {
    const char* cfg="node.cfg";
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

    Immortal server;
    server.registerRule(std::make_shared<DoudeZhu>());
    server.registerRule(std::make_shared<Mahjong>());
    server.registerRule(std::make_shared<Paohuzi>());

	server.run(cfg);
    server.set_timer(TIMER::TIMER_SEC, 1000);
    server.set_timer(TIMER::TIMER_MIN, 1000*60);
    server.set_timer(TIMER::TIMER_HOUR,1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);

    //DoudeZhu::test();
    while(true)msleep(1000000);

	return 0;
}

