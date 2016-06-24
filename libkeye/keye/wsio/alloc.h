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
#ifndef _wsio_alloc_h_
#define _wsio_alloc_h_

NS_KEYE_WS_BEGIN
// --------------------------------------------------------
class KEYE_API wsio_alloc{
public:
	typedef void	value_type;
	virtual			~wsio_alloc(){}
	virtual void*	allocate(size_t _Count)=0;
	virtual void	deallocate(void* _Ptr, size_t=0)=0;
};
// --------------------------------------------------------
NS_KEYE_WS_END
#endif // _io_alloc_h_