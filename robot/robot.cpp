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

robot::robot():ws_client(1, 1, 510) {
    sprintf(_buf, "hello i am websocket client\n");
}

void robot::on_open(svc_handler& sh) {
    KEYE_LOG("----on_open\n");
    auto len = strlen(_buf);
    sh.send(_buf, len);
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void robot::on_read(svc_handler& sh, void* buf, size_t sz) {
    handler.on_read(sh,buf,sz);
}

bool robot::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    KEYE_LOG("----on_timer %zd\n", id);
    bool ret = true;
    if (WRITE_TIMER == id) {
        auto len = strlen(_buf);
        sh.send(_buf, len);
    }
    return ret;
}

int main(int argc, char* argv[]) {
	const char* host="127.0.0.1";
	unsigned short port = 8899;
	robot client;
	client.connect(host, port);
	KEYE_LOG("++++client connect to %s:%d\n",host,port);
	std::getchar();

	return 0;
}
