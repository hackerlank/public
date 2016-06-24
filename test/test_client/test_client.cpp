// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <keye/htio/htio_demo.h>
#include "../protocol.h"

#ifdef WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

using namespace keye;

class Client:public RawClient{
	PackHelper<svc_handler,RawClient> packer;
public:
	virtual void	on_open(svc_handler& sh){
		MsgRaw msgRaw;
		msgRaw.mid=eMsg::MSG_RAW;
		sprintf(msgRaw.message,"This is raw");
		PacketWrapper pw(&msgRaw,sizeof(msgRaw));
		packer.send(sh,pw,false);
		sprintf(msgRaw.message,"Brilliant boy!");
		packer.send(sh,pw);
	}

	void on_message(keye::svc_handler& sh,keye::PacketWrapper& pw){
	}
};

int main(int argc, char* argv[]) {
	//	auto f=freopen("log.txt","w+",stdout);
	unsigned short port = 8899;
	const char* host="127.0.0.1";
	myclient(host, port, 4, 4);
	/*
	Client client;
	client.connect(host,port);

	printf("press any key to continue ...\n");
	_getch();
	*/
	return 0;
}
