// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: cache_provider.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _cache_provider_h_
#define _cache_provider_h_

namespace keye{
// --------------------------------------------------------
// cache_key and cache_data type
// --------------------------------------------------------
#pragma warning(disable:4200)
typedef size_t cache_key;
struct cache_data{
	size_t	length;		//data real length
	char	data[0];	//data real
};
// --------------------------------------------------------
// cache handler:notified by provider or cache
// --------------------------------------------------------
class KEYE_API cache_handler{
public:
	virtual			~cache_handler(){}
	virtual void	handle(cache_key,const void*,size_t){}
};
// --------------------------------------------------------
// cache provider,push pop and access line data in cache
// --------------------------------------------------------
class KEYE_API cache_provider{
public:
	virtual			~cache_provider(){}
	//push into cache
	virtual void	push(cache_key key,const void* str,size_t){}
	//pop from cache
	virtual void	pop(cache_key key){}
	//access cache
	virtual void	access(cache_key key,cache_handler* =nullptr){}
};
// --------------------------------------------------------
};
#endif