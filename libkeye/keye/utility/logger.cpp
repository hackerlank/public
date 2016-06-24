// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: logger.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <fstream>
#include <time.h>
#include <stdio.h>
#include "utility_fwd.h"

using namespace keye;

namespace keye{
class logger_impl{
public:
					logger_impl(const char*);
					~logger_impl();
	logger_impl&	operator<<(char v);
	logger_impl&	operator<<(int v);
	logger_impl&	operator<<(unsigned int v);
	logger_impl&	operator<<(long long v);
	logger_impl&	operator<<(unsigned long long v);
	logger_impl&	operator<<(const char* v);
	logger_impl&	operator<<(const std::string& v);
	logger_impl&	operator<<(logger_impl& (*)(logger_impl&));
	void			flush();
private:
	std::string		_file;
	FILE*			_fn;
};

logger& begl(logger& l){
    time_t t=time(NULL);
    tm* aTm=localtime(&t);
    /* YYYY	year
		MM   month (2 digits 01-12)
		DD   day (2 digits 01-31)
		HH   hour (2 digits 00-23)
		MM   minutes (2 digits 00-59)
		SS   seconds (2 digits 00-59) */
	char buf[32];
	sprintf(buf,"[%02d-%02d-%02d:%02d:%02d] ",aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
	return l<<(const char*)buf;
}
logger& endl(logger& l){
	return l<<'\n';
}
logger& endf(logger& l){
	l<<'\n';
	l.flush();
	return l;
}
logger& flush(logger& l){
	l.flush();
	return l;
}
};

logger_impl::logger_impl(const char* file)
:_fn(nullptr){
//	_fn=freopen(file,"w+",stdout);
	if(file){
		_file=file;
		_fn=fopen(file,"w+");
	}else _fn=stdout;
}
#include <iostream>
logger_impl::~logger_impl(){
	if(!_file.empty()&&_fn)
		fclose(_fn);
}

logger_impl& logger_impl::operator<<(char v){
	if(_fn)fprintf(_fn,"%c",v);
	return *this;
}

logger_impl& logger_impl::operator<<(int v){
	if(_fn)fprintf(_fn,"%d",v);
	return *this;
}

logger_impl& logger_impl::operator<<(unsigned int v){
	if(_fn)fprintf(_fn,"%u",v);
	return *this;
}

logger_impl& logger_impl::operator<<(long long v){
	if(_fn)fprintf(_fn,"%lld",v);
	return *this;
}

logger_impl& logger_impl::operator<<(unsigned long long v){
	if(_fn)fprintf(_fn,"%llu",v);
	return *this;
}

logger_impl& logger_impl::operator<<(const char* v){
	if(v)if(_fn)fprintf(_fn,"%s",v);
	return *this;
}

logger_impl& logger_impl::operator<<(logger_impl& (*_Pfn)(logger_impl&)){
	return (*_Pfn)(*this);
}

logger_impl& logger_impl::operator<<(const std::string& v){
	return (*this)<<v.c_str();
}
/*
logger_impl& logger_impl::log(const void* buf,size_t sz){
//	memcpy(_buf+_rp,buf,sz);
	return *this;
}
*/

void logger_impl::flush(){
	if(_fn)fflush(_fn);
}
// --------------------------------------------------------

logger::logger(const char* fn){
	_impl.reset(new logger_impl(fn));
}

logger& logger::operator<<(char v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(int v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(unsigned int v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(long long v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(unsigned long long v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(const char* v){
	if(_impl)(*_impl)<<v;
	return *this;
}

logger& logger::operator<<(const std::string& v){
	return (*this)<<v.c_str();
}

logger& logger::operator<<(logger& (*_Pfn)(logger&)){
	return (*_Pfn)(*this);
}

void logger::flush(){
	if(_impl)_impl->flush();
}
