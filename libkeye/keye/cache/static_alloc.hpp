// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: static_alloc.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _static_alloc_hpp_
#define _static_alloc_hpp_
namespace keye{
// --------------------------------------------------------
/* template static allocator,special for STL container:
	STL container need allocator with default ctor,
	some allocators must be construct explict and use like:
		keye_allocator kalloc(_Ax,_Count);
		vector<int,keye_allocator> vec(kalloc);
		typedef static_alloc<std::pair<const cache_key,cache_data*>,_Ax>	addr_alloc;
		typedef std::map<cache_key,cache_data*,std::less<cache_key>,addr_alloc>	addr_type;*/
// --------------------------------------------------------
//host allocator
struct static_alloc_base{static void*	_ax;};

template<typename _Ty,typename _Ax>
class static_alloc:public static_alloc_base{
public:
	typedef _Ty	value_type;
	typedef static_alloc<_Ty,_Ax>	_Myt;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	template<class _Other>
	struct rebind{
		typedef static_alloc<_Other,_Ax> other;
	};
	//default ctor
	static_alloc(){}
	//copyable ctor
	template<class _Other>
	static_alloc(const static_alloc<_Other,_Ax>&){}

	void construct(pointer _Ptr, const _Ty& _Val){
	::new (_Ptr) _Ty(_Val);
	}

	void destroy(pointer _Ptr){
		_Ptr->~_Ty();
	}

	size_t max_size() const{
		return 0;//_ax?static_cast<_Ax*>(_ax)->max_size():0;
	}

	value_type*		allocate(size_t sz){
		return _ax?(value_type*)static_cast<_Ax*>(_ax)->allocate(sizeof(_Ty)*sz):nullptr;
	}
	void		deallocate(value_type* p,size_t=0){
		if(_ax)static_cast<_Ax*>(_ax)->deallocate((value_type*)p,0);
	}
};
// --------------------------------------------------------
};// namespace
#endif
