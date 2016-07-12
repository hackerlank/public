// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: wsio_fwd.h
 *Desc		: a high throuput io library
 *Version	: 1.0
 *Program	: VicLiu
 *Date		: 2016-7-11
 */
// --------------------------------------------------------
#ifndef _wsio_fwd_h_
#define _wsio_fwd_h_
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

#ifndef KEYE_LOG
#	define KEYE_LOG printf
#endif

#ifndef s_ptr
#	define s_ptr std::shared_ptr
#endif

#include <keye/wsio/ws_handler.h>
#include <keye/wsio/ws_service.h>
#include <keye/wsio/work_handler.h>
// --------------------------------------------------------
#endif // _wsio_fwd_h_