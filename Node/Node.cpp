// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NodeFwd.h"

#ifdef WIN32
#include <conio.h>
#endif

using namespace keye;

enum TIMER:size_t{
    TIMER_SEC=100,
    TIMER_MIN,
    TIMER_HOUR,
    TIMER_DAY,
};

Node* Node::sNode=nullptr;

Node::Node(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size){
    sNode=this;
}

void Node::on_open(svc_handler&) {
//    KEYE_LOG("----on_open\n");
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void Node::on_close(svc_handler&) {
//    KEYE_LOG("----on_close\n");
}

void Node::on_read(svc_handler& sh, void* buf, size_t sz) {
    handler.on_read(sh,buf,sz);
}

void Node::on_write(svc_handler&, void*, size_t sz) {
//    KEYE_LOG("----on_write %zd\n",sz);
}

bool Node::on_timer(svc_handler&, size_t id, size_t milliseconds) {
    switch (id) {
        case TIMER::TIMER_SEC:
            for(auto game:gameRules)game.second->Tick();
            break;
        case TIMER::TIMER_MIN:
            break;
        case TIMER::TIMER_HOUR:
            break;
        case TIMER::TIMER_DAY:
            break;
        default:
            break;
    }
    return true;
}

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

    Node server;
	server.run(port,"127.0.0.1");
    server.set_timer(TIMER::TIMER_SEC, 1000);
    server.set_timer(TIMER::TIMER_MIN, 1000*60);
    server.set_timer(TIMER::TIMER_HOUR,1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);
	KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
