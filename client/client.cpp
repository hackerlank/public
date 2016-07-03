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
	unsigned short port = 8899;
	myserver(port, 4, 4);

	return 0;
}
