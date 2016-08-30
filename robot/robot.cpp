// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "robot_fwd.h"

#ifdef WIN32
#include <conio.h>
#else
//#include <curses.h>
#endif

using namespace keye;

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

robot* robot::sRobot=nullptr;
robot::robot(){
    sRobot=this;
}

// -------------------------------------------------------
robot::login_client::login_client():ws_client(1, 1, 510) {
}

void robot::login_client::on_open(svc_handler& sh) {
    KEYE_LOG("----on_open\n");
    spsh=sh();
    login();
}
void robot::login_client::login(){
    proto3::MsgCSLogin msg;
    msg.set_mid(eMsg::MSG_CS_LOGIN);
    msg.set_version(100);
    auto user=msg.mutable_user();
    user->set_account("robot");
    user->set_name("Robot");
    user->set_udid("vic's mac");
    PBHelper::Send(*spsh,msg);
}

void robot::login_client::on_read(svc_handler& sh, void* buf, size_t sz) {
    sRobot->handler.on_read(sh,buf,sz);
}

bool robot::login_client::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    KEYE_LOG("----on_timer %zd\n", id);
    bool ret = true;
    if (WRITE_TIMER == id) {
    }
    return ret;
}
// -------------------------------------------------------
robot::lobby_client::lobby_client():ws_client(1, 1, 510) {
}

void robot::lobby_client::on_open(svc_handler& sh) {
    KEYE_LOG("----on_open\n");
    spsh=sh();
    login();
}
void robot::lobby_client::login(){
    proto3::MsgCLEnter msg;
    msg.set_mid(eMsg::MSG_CL_ENTER);
    msg.set_version(100);
    msg.set_uid(sRobot->user.uid().c_str());
    PBHelper::Send(*spsh,msg);
}

void robot::lobby_client::on_read(svc_handler& sh, void* buf, size_t sz) {
    sRobot->handler.on_read(sh,buf,sz);
}

bool robot::lobby_client::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    KEYE_LOG("----on_timer %zd\n", id);
    bool ret = true;
    if (WRITE_TIMER == id) {
    }
    return ret;
}
// -------------------------------------------------------
robot::node_client::node_client():ws_client(1, 1, 510) {
}

void robot::node_client::on_open(svc_handler& sh) {
    KEYE_LOG("----on_open\n");
    spsh=sh();
    login();
}
void robot::node_client::login(){
    proto3::MsgCNEnter msg;
    msg.set_mid(eMsg::MSG_CN_ENTER);
    msg.set_version(100);
    msg.set_service(proto3::pb_enum::GAME_CARD);
    msg.set_uid(sRobot->user.uid().c_str());
    PBHelper::Send(*spsh,msg);
}

void robot::node_client::on_read(svc_handler& sh, void* buf, size_t sz) {
    sRobot->handler.on_read(sh,buf,sz);
}

bool robot::node_client::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    KEYE_LOG("----on_timer %zd\n", id);
    bool ret = true;
    if (WRITE_TIMER == id) {
    }
    return ret;
}

// -------------------------------------------------------
robot::http_client::http_client(){
}

void robot::http_client::on_read(svc_handler& sh, void* buf, size_t sz) {
    sRobot->handler.on_read(sh,buf,sz);
}

class A{
public:
    virtual void foo(){
        printf("A\n");
    }
    void bar(){
        foo();
    }
};
class B :public A{
public:
    void foo(){
        printf("B\n");
    }
};
// -------------------------------------------------------
int main(int argc, char* argv[]) {
    
	const char* host="127.0.0.1";
	unsigned short port = 8800;
	robot client;
//	client.login.connect(host, port);
//	KEYE_LOG("++++client connect to %s:%d\n",host,port);
    client.http.request(host,"hello",port);
    
    std::getchar();
    /*
    bool exit=false;
    while(!exit||!client.closed())
        switch(std::getchar()){
            case 'x':{
                client.close();
                exit=true;
                break;
            }
//            default:
  //              client.login();
        }
     */

	return 0;
}
