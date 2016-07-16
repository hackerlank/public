// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: Receiver.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2011-4-22
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <libvic/libvic_fwd.h>
#include <libvic/mysql_proxy/mysql_parameters.hpp>
#include <libvic/mysql_proxy/mysql_resultset.hpp>
#include <libvic/mysql_proxy/buf_set.h>
// --------------------------------------------------------
using namespace keye;
#define PR_ASSERT
parameter_impl::parameter_impl(size_t num,size_t id,void* pd)
:_index(0),_num((decltype(_num))num),_id(id),privdata(pd){
	_length=(unsigned short)(sizeof(_id)+sizeof(_num)+num*sizeof(e_field));
	_mysql_binds.reset(new MYSQL_BIND[num],std::default_delete<MYSQL_BIND[]>());
	memset(_mysql_binds.get(),0,sizeof(MYSQL_BIND)*num);
}

parameter_impl::parameter_impl(const void* buf)
:_index(0),_num(0),_length(sizeof(_id)+sizeof(_num)),_id(0){
	serialize(buf);
}

size_t parameter_impl::size()const{
	return _num;
}

size_t parameter_impl::id()const{
	return _id;
}
void parameter_impl::id(size_t n){
	_id=n;
}

void parameter_impl::_push_param_v(const void* val,e_field t,size_t len){
	if(_index<_num){
		MYSQL_BIND& mysql_bind=_mysql_binds.get()[_index++];
		mysql_bind.buffer_type=mysql_resultset_impl::_map(t);
		mysql_bind.buffer_length=(unsigned long)len;
		s_ptr<char> spbuf(new char[len],std::default_delete<char[]>());
		_mysql_binds_bufs.push_back(spbuf);
		mysql_bind.buffer=spbuf.get();
		memcpy(mysql_bind.buffer,val,len);
		_length+=(unsigned short)len;
	}
}

s_ptr<char> parameter_impl::deserialize(size_t& len)const{
	s_ptr<char> spbuf;
	if(len=_length){
		spbuf.reset(new char[len],std::default_delete<char[]>());
		char* buf=spbuf.get();
		//id
		buf_set(&buf,_id);
		//number
		buf_set(&buf,_num);
		//parameters
		LINFO("parameters(");
		for(unsigned char i=0;i<_num;++i){
			MYSQL_BIND& mysql_bind=_mysql_binds.get()[i];
			auto e=mysql_resultset_impl::_map(mysql_bind.buffer_type);
			//type
			buf_set(&buf,(unsigned char)e);
			//value
			if(EF_STRING==e||EF_BLOB==e)
				buf_set(&buf,(unsigned short)mysql_bind.buffer_length);
			memcpy(buf,mysql_bind.buffer,mysql_bind.buffer_length);
			buf+=mysql_bind.buffer_length;

			switch(e){
			case EF_BYTE:
				LINFO("%d,",*(unsigned char*)mysql_bind.buffer);
				break;
			case EF_SHORT:
				LINFO("%d,",*(unsigned short*)mysql_bind.buffer);
				break;
			case EF_INT:
				LINFO("%d,",*(unsigned int*)mysql_bind.buffer);
				break;
			case EF_LONGLONG:
				LINFO("%lld,",*(unsigned long long*)mysql_bind.buffer);
				break;
			case EF_STRING:
				LINFO("%s,",(unsigned char*)mysql_bind.buffer);
				break;
			case EF_BLOB:
				LINFO("*,");
				break;
			}
		}
		LINFO(").\n");
	}
	return spbuf;
}

void parameter_impl::serialize(const void* buf){
	if(buf){
		const char* ptr=(const char*)buf;
		unsigned char num=0;
		//id
		buf_get(_id,&ptr);
		//number
		buf_get(num,&ptr);
		if(!num)return;
		_num=num;
		_index=0;
		_length=sizeof(_id)+sizeof(_num)+_num*sizeof(e_field);
		_mysql_binds_bufs.clear();
		_mysql_binds.reset();

		//parameters
		_mysql_binds.reset(new MYSQL_BIND[_num],std::default_delete<MYSQL_BIND[]>());
		memset(_mysql_binds.get(),0,sizeof(MYSQL_BIND)*_num);

		for(unsigned char i=0;i<_num;++i){
			MYSQL_BIND& mysql_bind=_mysql_binds.get()[i];
			//type
			e_field e;
			buf_get((unsigned char&)e,&ptr);
			mysql_bind.buffer_type=mysql_resultset_impl::_map(e);
			//value
			switch(e){
			case EF_BYTE:
				mysql_bind.buffer_length=1;
				break;
			case EF_SHORT:
				mysql_bind.buffer_length=2;
				break;
			case EF_INT:
				mysql_bind.buffer_length=4;
				break;
			case EF_LONGLONG:
				mysql_bind.buffer_length=8;
				break;
			case EF_STRING:
			case EF_BLOB:
				buf_get((unsigned short&)mysql_bind.buffer_length,&ptr);
				break;
			}
			_push_param_v(ptr,e,mysql_bind.buffer_length);
			ptr+=mysql_bind.buffer_length;
		}
	}
}
// --------------------------------------------------------
// mysql parameter
// --------------------------------------------------------
mysql_parameter::mysql_parameter(size_t num,size_t id,void* privdata){
	_impl.reset(new parameter_impl(num,id,privdata));
}

mysql_parameter::mysql_parameter(const void* buf){
	_impl.reset(new parameter_impl(buf));
}

size_t mysql_parameter::size()const					{return _impl?_impl->size():0;}
size_t mysql_parameter::id()const					{return _impl?_impl->id():0;}
void mysql_parameter::id(size_t n)					{if(_impl)_impl->id(n);}
void mysql_parameter::push_byte(char val)			{if(_impl)_impl->_push_param(val,EF_BYTE);}
void mysql_parameter::push_short(short val)			{if(_impl)_impl->_push_param(val,EF_SHORT);}
void mysql_parameter::push_int(int val)				{if(_impl)_impl->_push_param(val,EF_INT);}
void mysql_parameter::push_longlong(long long val)	{if(_impl)_impl->_push_param(val,EF_LONGLONG);}
void mysql_parameter::push_string(const char* val)	{if(_impl)_impl->_push_param(val,EF_STRING,strlen(val)+1);}
void mysql_parameter::push_blob(const void* val,size_t len){if(_impl)_impl->_push_param(val,EF_BLOB,len);}
void mysql_parameter::serialize(const void* buf)	{if(_impl)_impl->serialize(buf);}
s_ptr<char> mysql_parameter::deserialize(size_t& len)const{
	s_ptr<char> spbuf;
	return _impl?_impl->deserialize(len):spbuf;
}
