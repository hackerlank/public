// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: cache.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/cache/cache_fwd.h>
#include <keye/cache/cache.hpp>

using namespace keye;
using namespace std;

cache_impl::cache_impl(cache_alloc& ax,cache_handler* hx,cache_provider* px)
	:cache_bctor<cache_alloc>(ax,hx,px)
	,_clients(0){
	INIT_LIST_HEAD(&_dirty_head);
	INIT_LIST_HEAD(&_free_head);
	INIT_LIST_HEAD(&_clean_head);
}

void cache_impl::push(cache_key key,const void* buf,size_t len){
	if(buf&&len){
		if(cache_data* data=_find(key))
			_update(data,buf,len);
		else{
			//allocate
			auto width=_line_width(len);
			cache_alloc& alloc=cache_bctor<cache_alloc>::_alloc;
			data=(cache_data*)alloc.allocate(width);
			if(data=_push(key,data,buf,len)){
				//push into cache and update
				IPC_GUARD_LOCK(_mutex);
				static_alloc_base::_ax=&alloc;	//prepare for _addr alloc
				_addr.insert(std::make_pair(key,data));
			}
		}
//		if(_provider)_provider->push(key,buf,len);
	}
}

void cache_impl::pop(cache_key key){
	if(cache_data* cd=_find(key)){
		IPC_GUARD_LOCK(_mutex);
		cache_line_t* line=CONTAINING_RECORD(cd,cache_line_t,data);
		auto& data=line->data;
		auto& head=line->head.head;
		//update
		line->head.age=0;
		//free
		list_del(&head);
		list_add_tail(&head,&_free_head);
	}
//	if(_provider)_provider->pop(key);
}

cache_data* cache_impl::access(cache_key key){
	cache_data* data=_find(key);
	if(!data&&_provider){
		//miss,call provider
		_provider->access(key,this);
	}else{
		//increate age
		_touch(data);
		//notify handler
		if(_handler)_handler->handle(key,data->data,data->length);
	}
	return data;
}

void cache_impl::flush(){
	std::map<cache_key,void*> dl,fl;
	list_head* pos=nullptr;
	{//make dirty clean
		IPC_GUARD_LOCK(_mutex);
		list_for_each(pos,&_dirty_head){
			auto& line=*(cache_line_t*)pos;
			auto& head=line.head;
			dl.insert(std::make_pair(head.id,&line.data));
		}
		list_replace_init(&_dirty_head,&_clean_head);
	}	
	{//recycle and return free
		IPC_GUARD_LOCK(_mutex);
		list_for_each(pos,&_free_head){
			auto& line=*(cache_line_t*)pos;
			auto& head=line.head;
			fl.insert(std::make_pair(head.id,pos));
		}
		INIT_LIST_HEAD(&_free_head);
	}
	//flush dirty
	if(_provider)for(auto i=dl.begin();i!=dl.end();++i){
		cache_key key=i->first;
		cache_data& data=*(cache_data*)i->second;
		_provider->push(key,data.data,data.length);
	}
	//deallocate
	for(auto i=fl.begin();i!=fl.end();++i){
		IPC_GUARD_LOCK(_mutex);
		cache_key key=i->first;
		_addr.erase(key);
		cache_alloc& alloc=cache_bctor<cache_alloc>::_alloc;
		alloc.deallocate(i->second);
		if(_provider)_provider->pop(key);
	}
}

void cache_impl::handle(cache_key key,const void* buf,size_t len){
	//push into cache
	if(buf)push(key,buf,len);
	//call cache handler
	if(_handler)_handler->handle(key,buf,len);
}

cache_data* cache_impl::_find(cache_key key){
	IPC_GUARD_LOCK(_mutex);
	auto it=_addr.find(key);
	return (it==_addr.end())?nullptr:it->second;
}

cache_data* cache_impl::_push(cache_key key,void* ptr,const void* _Buf,size_t _Count){
	if(ptr&&_Buf&&_Count){
		IPC_GUARD_LOCK(_mutex);
		auto& line=*(cache_line_t*)ptr;
		auto& head=line.head;
		auto& data=line.data;

		//line head
		INIT_LIST_HEAD(&head.head);
		head.id=key;
		head.width=sizeof(cache_line_t)+_Count;
		//line data
		data.length=_Count;
		//dirty
		list_add_tail(&head.head,&_dirty_head);

		//update cache line
		_update(line,_Buf,_Count);
		return &data;
	}
	return nullptr;
}

cache_data* cache_impl::_update(cache_data* ptr,const void* _Buf,size_t _Count){
	if(ptr&&_Buf&&_Count){
		IPC_GUARD_LOCK(_mutex);
		auto& line=*(cache_line_t*)CONTAINING_RECORD(ptr,cache_line_t,data);
		auto& head=line.head;
		auto& data=line.data;
		//modify,dirty
		list_del(&head.head);
		list_add_tail(&head.head,&_dirty_head);

		//update cache line
		_update(line,_Buf,_Count);
		return &data;
	}
	return nullptr;
}

void cache_impl::_touch(cache_data* data){
	if(data){
		cache_line_t* line=CONTAINING_RECORD(data,cache_line_t,data);
		++line->head.age;
	}
}

size_t cache_impl::_line_width(size_t data_len)const{
	return sizeof(cache_line_t)+data_len;
}

void cache_impl::_update(cache_line_t& line,const void* _Buf,size_t _Count){
	//update cache line
	auto& head=line.head;
	auto& data=line.data;
	if(_Count<=head.width){
		++head.age;
		memcpy(data.data,_Buf,_Count);
	}
}
// --------------------------------------------------------
// cache_base
// --------------------------------------------------------
cache_base::cache_base(cache_alloc& ax,cache_handler* hx,cache_provider* px){
	void* ptr=ax.allocate(sizeof(cache_impl));
	_impl=new (ptr)cache_impl(ax,hx,px);
}

void cache_base::push(cache_key key,const void* _Buf,size_t _Count){if(_impl)_impl->push(key,_Buf,_Count);}
void cache_base::pop(cache_key k){if(_impl)_impl->pop(k);}
cache_data* cache_base::access(cache_key k){return _impl?_impl->access(k):nullptr;}
void cache_base::flush(){if(_impl)_impl->flush();}
