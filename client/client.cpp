// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <keye/htio/htio_demo.h>

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

class MyClient :public ws_client{
public:
	MyClient(size_t ios = 1, size_t works = 1, size_t rb_size = 510) :ws_client(ios, works, rb_size) {
		sprintf(_buf, "hello i am websocket client\n");
	}
	virtual void	on_open(svc_handler& sh) {
		KEYE_LOG("----on_open\n");
		auto len = strlen(_buf);
		sh.send(_buf, len);
//		set_timer(WRITE_TIMER, WRITE_FREQ);
	}
	virtual void	on_read(svc_handler& sh, void* buf, size_t sz) {
		KEYE_LOG("----on_read %zd\n", sz);

		KEYE_LOG("read %d:%s\n", (int)sz, (char*)buf);
	}
	virtual bool	on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
		KEYE_LOG("----on_timer %zd\n", id);
		bool ret = true;
		if (WRITE_TIMER == id) {
			auto len = strlen(_buf);
			sh.send(_buf, len);
		}
		return ret;
	}
private:
	char _buf[WRITE_MAX];
};

int main(int argc, char* argv[]) {
	//const char* host = "192.168.99.100";
	const char* host="127.0.0.1";
	unsigned short port = 8899;
	//myclient<ws_client>(host, port);
	//myclient<service>(host,port);
	MyClient client;
	client.connect(host, port);
	KEYE_LOG("++++client connect to %s:%d\n",host,port);
	std::getchar();

	return 0;
}
