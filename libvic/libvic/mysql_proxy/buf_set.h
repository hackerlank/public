// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: logger.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#ifndef _buf_set_h
#define _buf_set_h

#include <string.h>

namespace keye{
// --------------------------------------------------------
// buffer help function
// --------------------------------------------------------
	template<typename T>
	void buf_set(char** buf,const T& val){
		if(buf&&*buf){
			memcpy(*buf,&val,sizeof(T));
			(*buf)+=sizeof(T);
		}
	}

	inline void buf_set(char** buf,const void* val,size_t len){
		if(buf&&*buf&&len){
			memcpy(*buf,val,len);
			(*buf)+=len;
		}
	}

	inline void buf_set(char** buf,const std::string& val){
		buf_set(buf,val.c_str(),val.length());
		buf_set(buf,'\0');
	}

	template<typename T>
	void buf_get(T& val,const char** buf){
		if(buf&&*buf){
			memcpy(&val,*buf,sizeof(T));
			(*buf)+=sizeof(T);
		}
	}

	inline void buf_get(void* val,size_t len,const char** buf){
		if(buf&&*buf&&len){
			memcpy(val,*buf,len);
			(*buf)+=len;
		}
	}

	inline void buf_get(std::string& val,const char** buf){
		if(buf&&*buf){
			val=*buf;
			(*buf)+=val.length()+1;
		}
	}
// --------------------------------------------------------
};// namespace
#endif
