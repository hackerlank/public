// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: shm_cache.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/cache/cache_fwd.h>

using namespace keye;
using namespace std;

namespace keye{
// --------------------------------------------------------
// the keye_allocator of shm_allocator
// --------------------------------------------------------
class shm_cache_ax:public cache_alloc{
public:
					shm_cache_ax(size_t sz,shm_allocator& sx):_ax(sz,sx){}
	virtual void*	allocate(size_t _Count){return _ax.allocate(_Count);}
	virtual void	deallocate(void* _Ptr, size_t=0){_ax.deallocate(_Ptr);}
private:
	keye_allocator<shm_allocator> _ax;
};
// --------------------------------------------------------
// shm_cache_impl
// --------------------------------------------------------
class shm_cache_impl:public cache_handler{
	typedef cache<shm_cache_ax> cache_t;
	typedef boost::interprocess::named_mutex nmutext_t;
public:
				shm_cache_impl(const char* name,size_t sz
					,cache_handler* =nullptr,cache_provider* =nullptr
					,const void* addr=(const void*)0x3000000);
				~shm_cache_impl();
	void		push(cache_key,const void* buf,size_t len);
	void		pop(cache_key key);
	cache_data*	access(cache_key key);
	void		flush();
	virtual void	handle(cache_key k,const void*,size_t);
private:
	void		_initialize();
	std::string		_name;			//name of shared memory object
	shm_allocator	_shm;			//shm allocator
	shm_cache_ax*	_alloc;			//cache allocator placed in shm
	cache_t*		_cache;			//cache placed in shm,without provider
	cache_handler*		_handler;	//the cache handler
	cache_provider*		_provider;	//the provider
	s_ptr<nmutext_t>	_mutex;
};};

shm_cache_impl::shm_cache_impl(const char* name,size_t sz,cache_handler* hx,cache_provider* px,const void* addr)
:_shm(name,sz,addr),_handler(hx),_provider(px),_name(name){
	_initialize();
}

shm_cache_impl::~shm_cache_impl(){
	nmutext_t::remove(_name.c_str());
}

void shm_cache_impl::push(cache_key key,const void* buf,size_t len){
	if(_cache)_cache->push(key,buf,len);
//	if(_provider)_provider->push(key,buf,len);
}

void shm_cache_impl::pop(cache_key key){
	if(_cache)_cache->pop(key);
//	if(_provider)_provider->pop(key);
}

cache_data* shm_cache_impl::access(cache_key key){
	cache_data* data=nullptr;
	if(_cache){
		data=_cache->access(key);
		if(!data&&_provider)
			//miss,call provider
			_provider->access(key,this);
		else
			//notify handler
			if(_handler)_handler->handle(key,data->data,data->length);
	}
	return data;
}

void shm_cache_impl::flush(){
	if(_cache)_cache->flush();
}

void shm_cache_impl::handle(cache_key key,const void* buf,size_t len){
	//push into cache
	if(buf)push(key,buf,len);
	//call cache handler
	if(_handler)_handler->handle(key,buf,len);
}

void shm_cache_impl::_initialize(){
	_name+="_mutex";	//make different to managed_shared_memory!!
	const char* name=_name.c_str();
	try{
		nmutext_t::remove(name);
		_mutex.reset(new nmutext_t(boost::interprocess::open_only,name));
	}catch(boost::interprocess::interprocess_exception& e){
		LOG(e.what());
		try{
			_mutex.reset(new nmutext_t(boost::interprocess::create_only,name));
		}catch(boost::interprocess::interprocess_exception& e){
			LOG("create named_mutex %s failed.%s\n",name,e.what());
			return;
		}
	}
	if(_mutex){
		boost::interprocess::scoped_lock<nmutext_t> lock(*_mutex);
		size_t color=_shm.max_size()-_shm.size();
		if(color==0x58){
			//allocate for allocator & cache object
			size_t sz_area=sizeof(shm_cache_ax)+sizeof(cache_t);
			char* area=(char*)_shm.allocate(sz_area);
			//ctor:place cache and cache allocator in shm
			_alloc=new (area)shm_cache_ax(_shm.size()&~0xff,_shm);
			_cache=new (area+sizeof(shm_cache_ax))cache_t(*_alloc);
		}else{
			//find
			_alloc=(shm_cache_ax*)((char*)_shm.address()+0x60);
			_cache=(cache_t*)((char*)_alloc+sizeof(shm_cache_ax));
		}
	}
}
// --------------------------------------------------------
// shm_cache_base
// --------------------------------------------------------
shm_cache_base::shm_cache_base(const char* name,size_t sz
	,cache_handler* hx,cache_provider* px
	,const void* addr){
	_impl.reset(new shm_cache_impl(name,sz,hx,px,addr));
}

void shm_cache_base::push(cache_key key,const void* _Buf,size_t _Count){if(_impl)_impl->push(key,_Buf,_Count);}
void shm_cache_base::pop(cache_key k){if(_impl)_impl->pop(k);}
cache_data* shm_cache_base::access(cache_key k){return _impl?_impl->access(k):nullptr;}
void shm_cache_base::flush(){if(_impl)_impl->flush();}
