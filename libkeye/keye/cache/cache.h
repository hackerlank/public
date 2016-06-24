// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: cache.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _cache_h_
#define _cache_h_

namespace keye{
// --------------------------------------------------------
// cache_base
// --------------------------------------------------------
class cache_impl;
class KEYE_API cache_base{
	template<typename> friend class cache_b;
	template<typename,typename,typename> friend class cache;
	template<typename,typename,typename,typename> friend class cache_a;
				cache_base(cache_alloc& ax,cache_handler* hx,cache_provider* px);
	void		push(cache_key k,const void* buf,size_t len);
	void		pop(cache_key k);
	cache_data*	access(cache_key k);
	void		flush();

	cache_impl*	_impl;
};
// --------------------------------------------------------
template<typename _Ax,typename _Hx,typename _Px,typename _Cx>
class KEYE_API cache_a{
protected:
				cache_a(_Ax& ax,_Hx* hx,_Px* px):_cx(ax,hx,px){}
	_Cx			_cx;
};
// --------------------------------------------------------
// cache_b,the real cache object.do not use it directly.
// --------------------------------------------------------
template<class _Cx>
class KEYE_API cache_b{
public:
				cache_b(_Cx& cx)	:_s(cx){}
	//push into cache
	void		push(cache_key k,const void* buf,size_t len)	{_s.push(k,buf,len);}
	//pop from cache
	void		pop(cache_key k)	{_s.pop(k);}
	//access cache,we provide 2 whole methos:
	//return a value if exists,and also notify handler if needed.
	cache_data*	access(cache_key k)	{return _s.access(k);}
	//make dirty clean,recycle free,call provider;
	//u may run on a timer thread
	void		flush()				{_s.flush();}
private:
	_Cx&		_s;
};
// --------------------------------------------------------
// cache,cache extend by allocator handler and provider
// --------------------------------------------------------
template<typename _Ax,typename _Hx=cache_handler,typename _Px=cache_provider>
class KEYE_API cache
	:public cache_a<_Ax,_Hx,_Px,cache_base>
	,public cache_b<cache_base>{
		typedef cache_a<_Ax,_Hx,_Px,cache_base>	_Mybase_a;
		typedef cache_b<cache_base>				_Mybase_b;
public:
	//ax:cache allocator;px:the provider
	cache(_Ax& ax,_Hx* hx=nullptr,_Px* px=nullptr)
		:_Mybase_a(ax,hx,px)
		,_Mybase_b(_Mybase_a::_cx){}
};
// --------------------------------------------------------
};// namespace
#endif // _cache_h_
