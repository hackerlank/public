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
void robot::http_client::set_uri(const char* uri){_uri=uri;}
void robot::http_client::request(eMsg mid,google::protobuf::MessageLite& msg){
    PBHelper::Request(*this,_uri.c_str(),msg,mid);
}
void robot::http_client::on_response(const http_parser& resp) {
    sRobot->handler.on_response(resp);
}
void robot::http_client::login(){
    auto mid=eMsg::MSG_CS_LOGIN;
    proto3::MsgCSLogin msg;
    msg.set_mid(mid);
    msg.set_version(100);
    auto user=msg.mutable_user();
    user->set_account("robot");
    user->set_name("Robot");
    user->set_udid("vic's mac");
    request(mid,msg);
    KEYE_LOG("----login\n");
}
void robot::http_client::enter_lobby(){
    auto mid=eMsg::MSG_CL_ENTER;
    proto3::MsgCLEnter msg;
    msg.set_mid(mid);
    msg.set_version(100);
    msg.set_uid(sRobot->user.uid().c_str());
    request(mid,msg);
    KEYE_LOG("----enter_lobby\n");
}
// -------------------------------------------------------
int main(int argc, char* argv[]) {
    int param=0;
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3){
            if(arg[0]=='-')switch(arg[1]){
                case 'u':{
                    auto a=&arg[2];
                    param=atoi(a);
                    break;
                }
                default:
                    break;
            }else{
            }
        }
    }
    
	const char* host="127.0.0.1";
    //unsigned short port = 8820;
    unsigned short port = 8080;
	robot client;
    auto skip_login=true;
    if(skip_login){
        srand((unsigned)time(nullptr));
        auto url=rand();
        if(param>0)url=param;
        //1145917110 1146875109
        //1146270057 1146606197,
        char uri[128];
        sprintf(uri,"ws://%s:%d/%d",host,port,url);
        robot::sRobot->node.connect(uri);
        KEYE_LOG("----connect to %s\n",uri);
        //ws://host:port/path
    }else{
        char uri[64];
        sprintf(uri,"http://%s:%d",host,port);
        client.http.set_uri(uri);
        client.http.login();
    }

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
