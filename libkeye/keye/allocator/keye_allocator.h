// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: keye_allocator.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _keye_allocator_h_
#define _keye_allocator_h_

namespace keye{
// --------------------------------------------------------
// growth strategy
// --------------------------------------------------------
class KEYE_API default_growth{
public:
	template<typename _Ax>size_t operator()(_Ax&){return 4<<20;}//4M
};
// --------------------------------------------------------
// no template implement class,bridge
// --------------------------------------------------------
class KEYE_API keye_impl:public std_allocator{
public:
				keye_impl(void*,size_t);
	void*		allocate(size_t);
	void		deallocate(void*);
	void*		address()const;
	size_t		max_size()const;
	size_t		size()const;
	bool		grow(void*,size_t);
	void**		shrink(size_t,size_t&);
private:
	size_t		_ph[8];
};
// --------------------------------------------------------
// preallocated plain scalar slab allocator
// --------------------------------------------------------
template<typename _Ax=std_allocator,typename _Gx=default_growth>
class KEYE_API keye_allocator:public uncopyable_allocator<keye_impl>{
public:
				keye_allocator(size_t size,_Ax& ax);
				~keye_allocator(){shrink(0);}
	void*		allocate(size_t _Count);
	//shrink space to _Size,only free spaces will be shrink
	void		shrink(size_t _Size=0);
private:
	_Ax&		_ax;	//host allocator
	keye_impl	_impl;	//implement allocator
};
// --------------------------------------------------------
#define LEAST_SIZE (PAGE_SIZE*4)	//at least 4 pages for prepare

template<typename _Ax,typename _Gx>
keye_allocator<_Ax,_Gx>::keye_allocator(size_t size,_Ax& ax)
	:_ax(ax)
	,_impl((void*)ax.allocate(size>=LEAST_SIZE?size:LEAST_SIZE),size>=LEAST_SIZE?size:LEAST_SIZE)
	,uncopyable_allocator(_impl){}

template<typename _Ax,typename _Gx>
void* keye_allocator<_Ax,_Gx>::allocate(size_t _Count){
	void* p=nullptr;
	if(_Count>PAGE_SIZE-16)
		LOG("Error:larger than max size(%d)\n",PAGE_SIZE-16);
	else if(!(p=_impl.allocate(_Count))){
		//allocate failed
		_Gx gx;
		auto sz=gx(*this);
		if(auto ptr=(void*)_ax.allocate(sz)){
			//growth from host
			_impl.grow(ptr,sz);
			//allocate again
			p=_impl.allocate(_Count);
		}
	}
	return p;
}

template<typename _Ax,typename _Gx>
void keye_allocator<_Ax,_Gx>::shrink(size_t sz){
	typedef typename _Ax::value_type alloc_value_type;
	size_t cnt=0;
	if(auto pp=_impl.shrink(sz,cnt)){
		for(size_t i=0;i<cnt;++i){
			auto ptr=(alloc_value_type*)pp[i];
			//shrink from host
			_ax.deallocate(ptr,0);
		}
		//deallocate temporary
		if(sz)deallocate(pp);
	}
}
// --------------------------------------------------------
};// namespace
#endif // _keye_allocator_h_
