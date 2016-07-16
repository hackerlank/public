// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: win32fixes.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#ifndef _win32fixes_h_
#define _win32fixes_h_

/*
 r3c do not support Win32 currently, also hiredis redefined some symbols
 conflict with ws32_2, so we need fix some functions :(
*/
#if(defined(_WIN32)||defined(_WIN64))
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#undef srandom
#undef random
#define srandom srand
#define random rand

static char buffer[16];
char* inet_ntoa(struct in_addr in){
	unsigned char *bytes=(unsigned char *)&in;
	snprintf(buffer,sizeof(buffer),"%d.%d.%d.%d",bytes[0],bytes[1],bytes[2],bytes[3]);
	return buffer;
}

unsigned long inet_addr(const char *cp){
	unsigned long a,b,c,d;
	sscanf(cp,"%ld.%ld.%ld.%ld",&a,&b,&c,&d);
	return d<<24|c<<16|b<<8|a;
}

size_t getpagesize(){ return 4096; }

__inline int gettimeofday(struct timeval *tp,void *tzp){
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year=wtm.wYear-1900;
	tm.tm_mon=wtm.wMonth-1;
	tm.tm_mday=wtm.wDay;
	tm.tm_hour=wtm.wHour;
	tm.tm_min=wtm.wMinute;
	tm.tm_sec=wtm.wSecond;
	tm.tm_isdst=-1;
	clock=mktime(&tm);
	tp->tv_sec=clock;
	tp->tv_usec=wtm.wMilliseconds*1000;

	return (0);
}
int replace_random(){
	unsigned int x=0;
	if(RtlGenRandom==NULL){
		// Load proc if not loaded
		HMODULE lib=LoadLibraryA("advapi32.dll");
		RtlGenRandom=(RtlGenRandomFunc)GetProcAddress(lib,"SystemFunction036");
		if(RtlGenRandom==NULL) return 1;
	}
	RtlGenRandom(&x,sizeof(unsigned int));
	return (int)(x>>1);
}
#endif
// --------------------------------------------------------
#endif // _win32fixes_h_
