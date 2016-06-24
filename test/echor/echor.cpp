#include "stdafx.h"
#include "htio_fx.h"

using namespace keye;

int main(int argc,char* argv[]){
//	auto f=freopen("log.txt","w+",stdout);
	const char* host="192.168.28.231";
	unsigned short port=8899;

	myserver(port,4,4);
	myclient(host,port,4,4);
	
	keye::pause();
	return 0;
}
