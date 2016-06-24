// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: static_allocator.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _static_allocator_h_
#define _static_allocator_h_
namespace keye{
// --------------------------------------------------------
/* template static allocator,special for STL container:
	STL container need allocator with default ctor,
	some allocators must be construct explict and use like:
		keye_allocator kalloc(_Ax,_Count);
		vector<int,keye_allocator> vec(kalloc);
		typedef static_allocator<std::pair<const cache_key,data_t*>,_Ax>	addr_alloc;
		typedef std::map<cache_key,data_t*,std::less<cache_key>,addr_alloc>	addr_type;*/
// --------------------------------------------------------
//host allocator
struct host_alloc{static void*	pointer;};

template<typename _Ty,typename _Ax>
class KEYE_API static_allocator{
public:
	typedef _Ty	value_type;
	typedef static_allocator<_Ty,_Ax>	_Myt;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef int difference_type;

	template<class _Other>
	struct rebind{
		typedef static_allocator<_Other,_Ax> other;
	};
	//default ctor
	static_allocator(){}
	//copyable ctor
	template<class _Other>
	static_allocator(const static_allocator<_Other,_Ax>&){}

	void construct(pointer _Ptr, const _Ty& _Val){
	::new (_Ptr) _Ty(_Val);
	}

	void destroy(pointer _Ptr){
		_Ptr->~_Ty();
	}

	size_t max_size() const{
		return host_alloc::pointer?static_cast<_Ax*>(host_alloc::pointer)->max_size():0;
	}

	value_type*		allocate(size_t sz){
		void* ax=host_alloc::pointer;
		return ax?(value_type*)static_cast<_Ax*>(ax)->allocate(sizeof(_Ty)*sz):nullptr;
	}
	void		deallocate(value_type* p,size_t=0){
		if(void* ax=host_alloc::pointer)
			static_cast<_Ax*>(ax)->deallocate((value_type*)p,0);
	}
};
// --------------------------------------------------------
};// namespace
#endif
