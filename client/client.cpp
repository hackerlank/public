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

int main(int argc, char* argv[]) {
    const char* host="192.168.99.100";
	unsigned short port = 8899;
	//myclient<ws_client>(host, port);
	myclient<service>(host,port);
	return 0;
}
