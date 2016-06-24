// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: cache.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _cache_hpp_
#define _cache_hpp_

#include "keye/cache/static_alloc.hpp"
namespace keye{
// --------------------------------------------------------
void* static_alloc_base::_ax=nullptr;
struct cache_line_t{
	struct head_t{
		list_head		head;
		cache_key		id;
		unsigned int	width:	16,	//data max length:1024
						//length:	MAX_ORDER_BIT,	//data real length
						prio:	8,	//priority than LRU
						age:	8;	//count for LRU
		size_t			stamp;		//time stamp
		//mutex_t		lock;
	};
	head_t				head;
	cache_data			data;
};

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address)-(unsigned long)(&((type*)0)->field)))
#endif//CONTAINING_RECORD
// --------------------------------------------------------
template<typename _Ax>
class cache_bctor{
public:
				cache_bctor(_Ax& ax,cache_handler* hx,cache_provider* px)
					:_alloc(ax)
					,_handler(hx)
					,_provider(px){
						static_alloc_base::_ax=&ax;
					}
protected:
	_Ax&		_alloc;
	cache_handler*		_handler;
	cache_provider*		_provider;
};

class cache_impl:public cache_bctor<cache_alloc>,public cache_handler{
public:
				cache_impl(cache_alloc& ax,cache_handler* hx,cache_provider* p=nullptr);
	void		push(cache_key,const void* buf,size_t len);
	void		pop(cache_key key);
	cache_data*	access(cache_key key);
	void		flush();
	virtual void	handle(cache_key k,const void*,size_t);
private:
	size_t		_line_width(size_t data_len)const;
	cache_data*	_find(cache_key key);	//find from _addr
	cache_data*	_update(cache_data* ptr,const void* _Buf,size_t _Count);
	void		_update(cache_line_t&,const void* _Buf,size_t _Count);
	cache_data*	_push(cache_key key,void* ptr,const void* _Buf,size_t _Count);
	void		_touch(cache_data* data);

	typedef static_alloc<std::pair<const cache_key,cache_data*>,cache_alloc>	addr_alloc;
	typedef std::map<cache_key,cache_data*,std::less<cache_key>,addr_alloc>	addr_type;
	addr_type	_addr;
	size_t		_clients;
	list_head	_dirty_head,_free_head,_clean_head;
	IPC_MUTEX(_mutex);
};
// --------------------------------------------------------
};// namespace
#endif // _cache_h_
