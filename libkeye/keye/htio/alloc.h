// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: htio_alloc.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _htio_alloc_h_
#define _htio_alloc_h_

namespace keye{
class KEYE_API htio_alloc{
public:
	typedef void value_type;
	virtual			~htio_alloc(){}
	virtual void*	allocate(size_t _Count)=0;
	virtual void	deallocate(void* _Ptr, size_t=0)=0;
};
// --------------------------------------------------------
};
#endif // _htio_alloc_h_