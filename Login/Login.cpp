// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LoginFwd.h"

#ifdef WIN32
#include <conio.h>
#endif

using namespace keye;

#ifdef WRITE_FREQ
#undef WRITE_FREQ
#define WRITE_FREQ 1000
#endif // WRITE_FREQ

slogin::slogin(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size) {}

void slogin::on_open(svc_handler&) {
    KEYE_LOG("----on_open\n");
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void slogin::on_close(svc_handler&) {
    KEYE_LOG("----on_open\n");
}

void slogin::on_read(svc_handler& sh, void* buf, size_t sz) {
    handler.on_read(sh,buf,sz);
}

void slogin::on_http(const http_parser& req,http_parser& resp){
    auto msgid=req.header("msgid");
    auto body=req.body();
    if(atoi(msgid)==eMsg::MSG_CN_ENTER){
        auto str=base64_decode(body);
        proto3::MsgCSLogin imsg;
        proto3::MsgSCLogin omsg;
        if(imsg.ParseFromString(str)){
            auto mid=eMsg::MSG_SC_LOGIN;
            omsg.set_uid("clusters");
            omsg.set_version(imsg.version()+1);
            omsg.set_ip("127.0.0.1");
            omsg.set_port(8810);
            omsg.set_result(proto3::pb_enum::SUCCEESS);
            omsg.set_mid(mid);
            
            char strmid[8];
            sprintf(strmid,"%d",mid);
            std::string str;
            omsg.SerializeToString(&str);
            str=base64_encode(str);

            resp.set_header("msgid",strmid);
            resp.set_body(str.c_str());
            resp.set_status(200,"OK");
        }else
            resp.set_status(500,"Internal error");
    }
}

void slogin::on_write(svc_handler&, void*, size_t sz) {
    KEYE_LOG("----on_write %zd\n",sz);
}

bool slogin::on_timer(svc_handler&, size_t id, size_t milliseconds) {
    KEYE_LOG("----on_timer %zd\n", id);
    if (FLOW_TIMER == id) {
    }
    return true;
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
	proto3::MsgSCLogin zi;

	redis_proxy redis;
	keye::PacketWrapper pw;
	PBHelper helper(pw);

    slogin server;
	server.run(port,"127.0.0.1");
	KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
