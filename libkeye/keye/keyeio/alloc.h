// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: io_alloc.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _io_alloc_h_
#define _io_alloc_h_

namespace keye{
class KEYE_API io_alloc{
public:
	typedef void value_type;
	virtual			~io_alloc(){}
	virtual void*	allocate(size_t _Count)=0;
	virtual void	deallocate(void* _Ptr, size_t=0)=0;
};
// --------------------------------------------------------
};
#endif // _io_alloc_h_