// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: shm_allocator.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#ifndef _shm_allocator_h_
#define _shm_allocator_h_

namespace keye{
// --------------------------------------------------------
// shared memory allocator
// --------------------------------------------------------
class shm_impl;
class KEYE_API shm_allocator:public std_allocator{
public:
				shm_allocator(const char*,size_t,const void* =0);
	void*		allocate(size_t);
	void		deallocate(void*,size_t=0);
	void*		address()const;
	size_t		max_size()const;
	size_t		size()const;
private:
	//must be nocopable
				shm_allocator(const shm_allocator&);
	shm_allocator&	operator=(const shm_allocator&);

	s_ptr<shm_impl>	_impl;
};
// --------------------------------------------------------
};// namespace
#endif // _shm_allocator_h_
