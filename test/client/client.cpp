#include "stdafx.h"
#include "htio_fx.h"

using namespace keye;

int main(int argc,char* argv[]){
	const char* host="192.168.28.231";
//	const char* host="127.0.0.1";
	unsigned short port=8899;
	myclient(host,port,4,4);
	keye::pause();
	return 0;
}
