// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: system.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-09
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/utility/utility_fwd.h>
#ifdef WIN32
#include <conio.h>
#else
#define getch getchar
//#include <curses.h>
#endif

namespace keye{
// --------------------------------------------------------
time_t ticker(){
	//milliseconds from history
	#ifdef WIN32
	struct _timeb tb;
	_ftime(&tb);
	#else
	struct timeb tb;
	ftime(&tb);
	#endif
	return tb.time*1000+tb.millitm;
}

void pause(){
	printf("press any key to continue...\n");
	getch();
}

bool is_bigendian(){
	const int endian=1;
	return ((*(char*)&endian)==0);
}
    
void msleep(int millisecond){
#if (defined(_WIN32)||defined(_WIN64))
    Sleep(millisecond);
#else
    struct timespec ts = { millisecond / 1000, (millisecond % 1000) * 1000000 };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
#endif
}

// --------------------------------------------------------
};//namespace keye
