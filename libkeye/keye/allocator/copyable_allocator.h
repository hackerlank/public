// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: copyable_allocator.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _copyable_allocator_h_
#define _copyable_allocator_h_

namespace keye{
// --------------------------------------------------------
// stl std allocator,with deallocate operator
// --------------------------------------------------------
class KEYE_API std_allocator:public std::allocator<char>{
public:
	//deallocator operator,not virtual
	void		operator()(void* _Ptr)			{deallocate((value_type*)_Ptr,0);}
};
// --------------------------------------------------------
// template copyable allocator,reference of other allocator,no ctor and dtor
// --------------------------------------------------------
template<typename _Ax>
class KEYE_API copyable_allocator:public std_allocator{
public:
				copyable_allocator(_Ax& ax)		:_alloc(ax){}
	void*		allocate(size_t _Count)			{return _alloc.allocate(_Count);}
	void		deallocate(void* _Ptr, size_t=0){_alloc.deallocate(_Ptr);}
	void*		address()const					{return _alloc.address();}
	//size total
	size_t		max_size()const					{return _alloc.max_size();}
	//size free
	size_t		size()const						{return _alloc.size();}
private:
	_Ax&		_alloc;	//host allocator
};
// --------------------------------------------------------
// template uncopyable allocator,reference of other allocator,no ctor and dtor
// --------------------------------------------------------
template<typename _Ax>
class KEYE_API uncopyable_allocator:public copyable_allocator<_Ax>{
public:
				uncopyable_allocator(_Ax& ax):copyable_allocator<_Ax>(ax){}
private:
	//must be nocopable
				uncopyable_allocator(const uncopyable_allocator&);
	uncopyable_allocator&	operator=(const uncopyable_allocator&);
};
// --------------------------------------------------------
};// namespace
#endif
