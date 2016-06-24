#include "stdafx.h"
#include "htio_fx.h"

using namespace keye;

int main(int argc,char* argv[]){
//	auto f=freopen("log.txt","w+",stdout);
	unsigned short port=8899;
	myserver(port,4,4);

//	mysql_test();
//	cache_test();
//	alloc_test();
	
	keye::pause();
	return 0;
}
