// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: alloc_fwd.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-3-7
 */
// --------------------------------------------------------
#ifndef _alloc_fwd_h_
#define _alloc_fwd_h_
// --------------------------------------------------------
#ifndef KEYE_API
#if(defined(_WIN32)||defined(_WIN64))&&_WINDLL
	#if KEYE_EXPORT
		#define KEYE_API __declspec(dllexport)
	#else
		#define KEYE_API __declspec(dllimport)
	#endif
#else
	#define KEYE_API
#endif
#endif //KEYE_API

#ifndef LOG
#	define LOG printf
#endif

#ifndef s_ptr
#	define s_ptr std::shared_ptr
#endif

#define MIN_ORDER_BIT	(4)					//16=1<<4
#define MAX_ORDER_BIT	(12)				//4096=1<<12
#define PAGE_SIZE		(1<<MAX_ORDER_BIT)	//4k=0x1000
#define ALIGN_MASK		(~(PAGE_SIZE-1))	//4k aligned,0xfffff000

#include <keye/allocator/list_head.h>
#include <keye/allocator/copyable_allocator.h>
#include <keye/allocator/shm_allocator.h>
#include <keye/allocator/keye_allocator.h>
#include <keye/allocator/static_allocator.h>
// --------------------------------------------------------
#endif // _alloc_fwd_h_
