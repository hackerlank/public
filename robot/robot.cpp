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
robot::robot()
:key(-1)
,game_id(-1){
    sRobot=this;
}
// -------------------------------------------------------
robot::node_client::node_client():ws_client(1, 1, 510) {
}

void robot::node_client::on_open(svc_handler& sh) {
    KEYE_LOG("on_open\n");
    spsh=sh();
    login();
}
void robot::node_client::login(){
    proto3::MsgCNEnter msg;
    msg.set_mid(proto3::pb_msg::MSG_CN_ENTER);
    msg.set_version(100);
    msg.set_service(proto3::pb_enum::GAME_CARD);
    msg.set_uid(sRobot->user.uid().c_str());
    PBHelper::Send(*spsh,msg);
}

void robot::node_client::on_read(svc_handler& sh, void* buf, size_t sz) {
    sRobot->handler.on_read(sh,buf,sz);
}

bool robot::node_client::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    KEYE_LOG("on_timer %zd\n", id);
    bool ret = true;
    if (WRITE_TIMER == id) {
    }
    return ret;
}

// -------------------------------------------------------
void robot::http_client::set_uri(const char* uri){_uri=uri;}
void robot::http_client::request(proto3::pb_msg mid,google::protobuf::MessageLite& msg){
    PBHelper::Request(*this,_uri.c_str(),msg,mid);
}
void robot::http_client::on_response(const http_parser& resp) {
    sRobot->handler.on_response(resp);
}
void robot::http_client::login(){
    auto mid=proto3::pb_msg::MSG_CS_LOGIN;
    proto3::MsgCLLogin msg;
    msg.set_mid(mid);
    msg.set_version(100);
    auto user=msg.mutable_user();
    user->set_account("robot");
    user->set_name("Robot");
    user->set_udid("vic's mac");
    request(mid,msg);
    KEYE_LOG("login\n");
}
void robot::http_client::enter_lobby(){
    auto mid=proto3::pb_msg::MSG_CL_LOBBY;
    proto3::MsgCLLobby msg;
    msg.set_mid(mid);
    msg.set_version(100);
    msg.set_uid(sRobot->user.uid().c_str());
    request(mid,msg);
    KEYE_LOG("enter_lobby\n");
}
// -------------------------------------------------------
int main(int argc, char* argv[]) {
    const char* host="127.0.0.1";
    //unsigned short port = 8820;
    unsigned short port = 8080;

    int param=0;
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3){
            if(arg[0]=='-')switch(arg[1]){
                case 'u':{
                    param=atoi(&arg[2]);
                    break;
                }
                case 'p':{
                    port=atoi(&arg[2]);
                    break;
                }
                default:
                    break;
            }else{
            }
        }
    }
    
    /*
     websocket uri format ws://host:port/path
     game_id=game_key:game_index(xxxyyyyy),use game_key as ws path
     client select a random number as game_key to connect to Node
     Node create game and asign game_id with game_key:game_index
     client B game_id to connect to same Node and join game
     */
	robot client;
    auto skip_login=false;
    if(skip_login){
        int key=0;
        if(param>0){
            //join game,specify node id
            client.game_id=param;
            key=param/proto3::pb_enum::DEF_MAX_GAMES_PER_NODE;
        }else{
            //create game,rand node id
            srand((unsigned)time(nullptr));
            key=rand();
        }
        key=key%proto3::pb_enum::DEF_MAX_NODES;
        client.key=key;
        char uri[128];
        sprintf(uri,"ws://%s:%d/%d",host,port,key);
        robot::sRobot->node.connect(uri);
        KEYE_LOG("connect to %s\n",uri);
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
