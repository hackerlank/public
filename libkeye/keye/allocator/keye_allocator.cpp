// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: keye_allocator.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/allocator/alloc_fwd.h>

using namespace std;
using namespace keye;
// --------------------------------------------------------
/* chain_alloc and unit_t are thread safe while using _mutex,
	but both cause 50% of inefficiency;
	while using chain_alloc,_mutex of unit_t can be closed.
*/
// --------------------------------------------------------
namespace keye{
void* host_alloc::pointer=nullptr;

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address)-(unsigned long)(&((type*)0)->field)))
#endif//CONTAINING_RECORD
// --------------------------------------------------------
// slab head structure
// --------------------------------------------------------
class unit_t;
struct slab_t{
	//internal slab sturct
	list_head		head;		//linker
	unit_t*			unit;		//allocator unit pointer
	unsigned char	order;		//order=log2(width)
	char			data[0];	//dummy holder
};
// --------------------------------------------------------
// unit allocator
// --------------------------------------------------------
class unit_t{
public:
				unit_t(void*,size_t);
	void*		allocate(size_t _Count);
	void		deallocate(void* _Ptr);
	void*		address()const;
	size_t		max_size()const;	//size total
	size_t		size()const;		//size free
private:
	void		_prepare();
	bool		_inc_free(size_t order,size_t _Count=1);
	size_t		_log2(size_t);

	struct slab_head{
		size_t		length;
		list_head	head;
	};
	slab_head	_free_l[MAX_ORDER_BIT-MIN_ORDER_BIT+1],
				_alloc_l[MAX_ORDER_BIT-MIN_ORDER_BIT+1];
	size_t		_capacity;				//total useful
	char		*_begin,*_curr,*_end;	//allocate pointer
	IPC_MUTEX	(_mutex);	//ipc mutex is almost as fast as mutex
};
// --------------------------------------------------------
// unit allocator chain
// --------------------------------------------------------
class chain_alloc{
public:
				chain_alloc(void*,size_t);
	void*		allocate(size_t _Count);
	void		deallocate(void* _Ptr);
	void*		address()const;
	size_t		max_size()const;	//size total
	size_t		size()const;		//size free
	bool		grow(void*,size_t);	//grow and redirect buffer
	void**		shrink(size_t,size_t&);//retrieve chain for deallocate
private:
	size_t		_sz_chain,_len_chain;
	unit_t**	_chain;		//allocators chain
	IPC_MUTEX	(_mutex);	//ipc mutex is almost as fast as mutex
	enum		{GROWTH=64};
};
};
// --------------------------------------------------------
chain_alloc::chain_alloc(void* p,size_t size)
:_sz_chain(0),_len_chain(0)
,_chain(nullptr){
	grow(p,size);
}

void* chain_alloc::allocate(size_t _Count){
	IPC_GUARD_LOCK(_mutex);
	//from head to tail
	void* p=nullptr;
	for(size_t i=0;i<_sz_chain;++i){
		auto pu=(unit_t*)_chain[i];
		if(p=pu->allocate(_Count))
			break;
	}
	return p;
}

void chain_alloc::deallocate(void* _Ptr){
	auto u=CONTAINING_RECORD(_Ptr,slab_t,data);
	IPC_GUARD_LOCK(_mutex);
	u->unit->deallocate(_Ptr);
}

void* chain_alloc::address()const{
	return _sz_chain>0?_chain[0]:nullptr;
}

size_t chain_alloc::max_size()const{
	size_t sz=0;
	for(size_t i=0;i<_sz_chain;++i)
		if(auto pu=(unit_t*)_chain[i])
			sz+=pu->max_size();
	return sz;
}

size_t chain_alloc::size()const{
	size_t sz=0;
	for(size_t i=0;i<_sz_chain;++i)
		if(auto pu=(unit_t*)_chain[i])
			sz+=pu->size();
	return sz;
}

bool chain_alloc::grow(void* p,size_t size){
	auto color=sizeof(unit_t);
	char* start=(char*)p+color;
	auto u=new (p) unit_t(start,size-color);

	IPC_GUARD_LOCK(_mutex);
	//alloc chain by unit0
	if(_sz_chain==_len_chain){
		//need grow
		auto old_sz=_sz_chain;
		auto old_chain=_chain;
		_len_chain+=GROWTH;
		size_t sz=_len_chain*sizeof(void*);
		//allocate new chain by unit
		auto p=u->allocate(sz);
		_chain=(unit_t**)p;
		//add unit at head of chain
		_chain[0]=u;

		if(old_chain){
			//copy and deallocate old chain
			memcpy(&_chain[1],&old_chain[0],sizeof(_chain[0])*old_sz);
			auto pu=(unit_t*)_chain[1];
			pu->deallocate((void*)old_chain);
		}
	}else
		_chain[_sz_chain]=u;
	++_sz_chain;
/*
	LOG("chain at growth: len=%d,",_sz_chain);
	for(size_t i=0;i<_sz_chain;++i)LOG("0x%x,",_chain[i]);
	LOG("\n");
*/
	return true;
}

void** chain_alloc::shrink(size_t sz,size_t& cnt){
	void** ret=nullptr;
	cnt=0;
	if(0==sz){
		cnt=_sz_chain;
		ret=(void**)_chain;
	}else if(_sz_chain>1&&sz<max_size()){
		size_t need=_sz_chain*sizeof(void*);
		ret=(void**)allocate(need);

		IPC_GUARD_LOCK(_mutex);
		//from tail to head
		for(int i=_sz_chain-1;i>=1;--i){
			auto pu=(unit_t*)_chain[i];
			LOG("===shrink 0x%x: %d/%d",(size_t)pu,pu->size(),pu->max_size());
			if(pu->size()==pu->max_size()){
				ret[cnt++]=(void*)pu;
				_chain[i]=nullptr;
			}
			if(sz>=max_size())
				break;
		}
		//rebuild chain
		if(cnt){
			auto tmp=new unit_t* [_sz_chain];
			memcpy(tmp,_chain,_sz_chain*sizeof(unit_t*));
			size_t new_len=_sz_chain-cnt;
			if(_len_chain-new_len>=GROWTH){
				while(_len_chain-new_len>=GROWTH)
					_len_chain-=GROWTH;
				auto au=CONTAINING_RECORD(_chain,slab_t,data);
				auto u=au->unit;
				u->deallocate(_chain);
				_chain=(unit_t**)u->allocate(_len_chain*sizeof(void*));
			}
			new_len=_sz_chain;
			_sz_chain=0;
			for(size_t i=0;i<new_len;++i)
				if(tmp[i])_chain[_sz_chain++]=tmp[i];
			delete[] tmp;
		}
	}
	return ret;
}
// --------------------------------------------------------
inline unit_t::unit_t(void* ptr,size_t size)
:_capacity(size)
,_begin(nullptr)
,_curr(nullptr)
,_end(nullptr){
	if(ptr){						//&&(_capacity&=ALIGN_MASK),4096=0x1000 aligned
		//reset position
		_begin=_curr=(char*)ptr;	//(size_t(ptr)&ALIGN_MASK);
		_end=_begin+_capacity;
		_prepare();
	}
}

inline void* unit_t::allocate(size_t _Count){
	const size_t max_sz=(1<<MAX_ORDER_BIT)-sizeof(slab_t);
	if(_Count<=max_sz){
//		IPC_GUARD_LOCK(_mutex);
		auto order=_log2(_Count+sizeof(slab_t));	//slab_t+data
		size_t idx=order-MIN_ORDER_BIT;
		auto& free_slab=_free_l[idx];
		auto& alloc_slab=_alloc_l[idx];
		if(free_slab.length==0){
			//increase free list about alloc.length*12.5%
			size_t len=alloc_slab.length>>3;	//n/8
			_inc_free(order,len>8?len:8);
		}
		if(free_slab.length>0){
			auto slab=(slab_t*)free_slab.head.prev;
			//pop from free list,and push back alloc list
			list_del(&slab->head);
			list_add_tail(&slab->head,&alloc_slab.head);
			++alloc_slab.length;
			--free_slab.length;

			return slab->data;
		}
	}else
		LOG("More larger than max slab(%d)\n",max_sz);
	return nullptr;
}

inline void unit_t::deallocate(void* _Ptr){
	if(_Ptr){
//		IPC_GUARD_LOCK(_mutex);
		auto slab=CONTAINING_RECORD(_Ptr,slab_t,data);
		size_t idx=slab->order-MIN_ORDER_BIT;
		auto& free_slab=_free_l[idx];
		auto& alloc_slab=_alloc_l[idx];

		list_del(&slab->head);
		list_add_tail(&slab->head,&free_slab.head);
		--alloc_slab.length;
		++free_slab.length;
	}
}

inline void unit_t::_prepare(){
	//initialize the slab list
	size_t idx=MAX_ORDER_BIT-MIN_ORDER_BIT+1;
	list_head* head=nullptr;
	for(size_t i=0;i<idx;++i){
		auto& free_slab=_free_l[i];
		auto& alloc_slab=_alloc_l[i];
		free_slab.length=alloc_slab.length=0;
		INIT_LIST_HEAD(&free_slab.head);
		INIT_LIST_HEAD(&alloc_slab.head);
	}
	//preallocate 1/4,min at 1024 pages
	size_t pre_alloc=_capacity;
		if(_capacity>PAGE_SIZE*1024)pre_alloc/=4;
	size_t complete_order_size=0;
	for(int i=MIN_ORDER_BIT;i<=MAX_ORDER_BIT;++i)
		complete_order_size+=1<<i;
	size_t pre_alloc_slabs=pre_alloc/complete_order_size;

	//make slabs
	for(size_t j=0,jj=MAX_ORDER_BIT-MIN_ORDER_BIT+1;j<jj;++j){
		size_t order=j+MIN_ORDER_BIT;
		_inc_free(order,pre_alloc_slabs);
	}
//	LOG("Pre alloc %d slabs,%dk( complete_order_size=%d )\n",pre_alloc_slabs*(MAX_ORDER_BIT-MIN_ORDER_BIT+1),pre_alloc/1024,complete_order_size);
}

inline bool unit_t::_inc_free(size_t order,size_t _Count){
	if(order>MAX_ORDER_BIT){
		LOG("More larger than max slab(%d)\n",1<<MAX_ORDER_BIT);
		return false;
	}
	size_t width=1<<order,
		free_count=(_end-_curr)/width;
	if(_Count>free_count)_Count=free_count;
	if(_Count>0){
		size_t idx=order-MIN_ORDER_BIT;
		auto& free_slab=_free_l[idx];
		auto curr=_curr;
		for(size_t i=0;i<_Count;++i){
			auto slab=(slab_t*)_curr;
			slab->order=order;
			slab->unit=this;
			list_add_tail(&slab->head,&free_slab.head);
			_curr+=width;
		}
		free_slab.length+=_Count;
//		LOG("Inc alloc slabs(%dk=%d * %d)\n",(_curr-curr)/1024,1<<order,_Count);

		return true;
	}
	return false;
}

inline void* unit_t::address()const{
	return _begin;
}

inline size_t unit_t::max_size()const{
	return _capacity;
}

inline size_t unit_t::size()const{
	size_t sz=_end-_curr;
	for(size_t i=0,ii=MAX_ORDER_BIT-MIN_ORDER_BIT+1;i<ii;++i){
		auto& free_slab=_free_l[i];
		size_t order=i+MIN_ORDER_BIT;
		sz+=free_slab.length*(1<<order);
	}
	return sz;
}

inline size_t unit_t::_log2(size_t x){
	//log2:__asm__ __volatile__(¡°bsrl %1, %%eax¡± : ¡°=a¡±(ret) : ¡°m¡±(x));
	size_t X=1<<MAX_ORDER_BIT;
	for(int i=MAX_ORDER_BIT;i>=0;--i){
		if(x&X){
			if(x^X)++i;
			return i;
		}
		X>>=1;
	}
	return 0;
}
// --------------------------------------------------------
// keye_impl
// --------------------------------------------------------
keye_impl::keye_impl(void* ptr,size_t size){
	memset(_ph,0,sizeof(_ph));
	auto xx=sizeof(chain_alloc);
	assert(sizeof(chain_alloc)<=sizeof(_ph));
	if(sizeof(chain_alloc)<=sizeof(_ph))
		new(&_ph) chain_alloc(ptr,size);
}
void*	keye_impl::allocate(size_t _Count)		{return *_ph?((chain_alloc*)_ph)->allocate(_Count):nullptr;}
void	keye_impl::deallocate(void* Ptr)		{if(*_ph)((chain_alloc*)_ph)->deallocate(Ptr);}
void*	keye_impl::address()const				{return *_ph?(char*)((chain_alloc*)_ph)->address():0;}
size_t	keye_impl::max_size()const				{return *_ph?((chain_alloc*)_ph)->max_size():0;}
size_t	keye_impl::size()const					{return *_ph?((chain_alloc*)_ph)->size():0;}
bool	keye_impl::grow(void* ptr,size_t sz)	{return *_ph?((chain_alloc*)_ph)->grow(ptr,sz):false;}
void**	keye_impl::shrink(size_t sz,size_t& n)	{return *_ph?((chain_alloc*)_ph)->shrink(sz,n):nullptr;}
// --------------------------------------------------------
static void list_print(list_head* head){
	if(head){
		list_head* pos=nullptr;
		LOG("head(%d)",(size_t)head);
		list_for_each(pos,head)
			LOG("->next(%x)",(size_t)pos);
		LOG("\nhead(%d)",(size_t)head);
		list_for_each_prev(pos,head)
			LOG("->prev(%x)",(size_t)pos);
		LOG("\n");
	}
}
