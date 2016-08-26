// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "slogin_fwd.h"

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

// --------------------------------------------------------
// PBHelper: protobuf helper
// --------------------------------------------------------
class PBHelper{
public:
	static const size_t send_buffer_size=2048;
	PBHelper(keye::PacketWrapper& pw):_pw(pw){}

	bool Parse(google::protobuf::MessageLite& msg){
		return msg.ParseFromArray(_pw.data,(int)_pw.length);
	}

	eMsg Id(){
		proto3::MsgBase mt;
		mt.ParseFromArray(_pw.data,4);
		return (eMsg)mt.mid();
	}

	static void Send(keye::svc_handler& sh,google::protobuf::MessageLite& msg){
		auto bytes=msg.ByteSize();
		assert(bytes<send_buffer_size);		//large message
		char buffer[send_buffer_size];
		if(msg.SerializeToArray(buffer,bytes)){
			proto3::MsgBase mr;
			if(mr.ParseFromArray(buffer,bytes)){
				assert(mr.mid()<=0);

				keye::HeadPacker packer;
				keye::PacketWrapper pw(buffer,bytes);
				packer<<pw;
				packer>>pw;
				sh.send(pw.data,pw.length);
				return;
			}
		}
		assert(false);
	}
	//make compiler happy
	void	on_message(keye::svc_handler&,keye::PacketWrapper&){}
private:
	keye::PacketWrapper& _pw;
};

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
    unsigned short port = 8899;
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
	proto3::ZoneInfo zi;
	myserver<ws_service>(port, 4, 4);
	//myserver<service>(port, 4, 4);

	redis_proxy redis;
	keye::PacketWrapper pw;
	PBHelper helper(pw);
	return 0;
	slogin server;
	server.run(port,"127.0.0.1");
	KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
