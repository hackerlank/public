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
	//myserver<ws_service>(port, 4, 4);
	myserver<service>(port, 4, 4);
	return 0;
}
