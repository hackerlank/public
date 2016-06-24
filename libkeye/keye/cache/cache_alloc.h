// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: cache_alloc.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _cache_alloc_h_
#define _cache_alloc_h_

namespace keye{
// --------------------------------------------------------
// allocator for cache
// --------------------------------------------------------
class KEYE_API cache_alloc{
public:
	virtual			~cache_alloc(){}
	virtual void*	allocate(size_t _Count)=0;
	virtual void	deallocate(void* _Ptr, size_t=0)=0;
};
// --------------------------------------------------------
};
#endif