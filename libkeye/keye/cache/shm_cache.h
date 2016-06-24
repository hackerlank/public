// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: shm_cache.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-7
 */
// --------------------------------------------------------
#ifndef _shm_cache_
#define _shm_cache_
namespace keye{
// --------------------------------------------------------
// shm_cache_base
// --------------------------------------------------------
class shm_cache_impl;
class KEYE_API shm_cache_base{
	template<typename> friend class cache_b;
	template<typename,typename> friend class shm_cache;
	template<typename,typename,typename> friend class cache_a1;
				shm_cache_base(const char* name,size_t sz
					,cache_handler*,cache_provider*,const void* addr);
	void		push(cache_key k,const void* buf,size_t len);
	void		pop(cache_key k);
	cache_data*	access(cache_key k);
	void		flush();

	s_ptr<shm_cache_impl>	_impl;
};

template<typename _Hx,typename _Px,typename _Cx>
class KEYE_API cache_a1{
protected:
				cache_a1(const char* name,size_t sz
					,_Hx* hx,_Px* px,const void* addr)
					:_cx(name,sz,hx,px,addr){}
	_Cx			_cx;
};
// --------------------------------------------------------
// shm_cache,cache in shared memory
// --------------------------------------------------------
template<typename _Hx=cache_handler,typename _Px=cache_provider>
class KEYE_API shm_cache
	:public cache_a1<_Hx,_Px,shm_cache_base>
	,public cache_b<shm_cache_base>{
		typedef cache_a1<_Hx,_Px,shm_cache_base>	_Mybase_a;
		typedef cache_b<shm_cache_base>				_Mybase_b;
public:
	/* name:the name of shared memory object,
	sz:the size of shared memory,
	px:the provider,
	addr:base address of shared memory */
	shm_cache(const char* name
		,size_t sz
		,_Hx* hx=nullptr
		,_Px* px=nullptr
		,const void* addr=(const void*)0x3000000)
		:_Mybase_a(name,sz,hx,px,addr)
		,_Mybase_b(_Mybase_a::_cx){}
};
// --------------------------------------------------------
};// namespace
#endif
