// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: htio_fwd.h
 *Desc		: a high throuput io library
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-3-7
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

#ifndef NS_KEYE
	#define NS_KEYE_BEGIN		namespace keye{
	#define NS_KEYE_END			}
	#define USING_NS_KEYE		using namespace keye;
	#define NS_KEYE_WS_BEGIN	namespace keye{namespace ws{
	#define NS_KEYE_WS_END		}}
	#define USING_NS_KEYE_WS	using namespace keye::ws;
#endif

#include <keye/wsio/alloc.h>
#include <keye/wsio/svc_handler.h>
#include <keye/wsio/work_handler.h>
#include <keye/wsio/service.h>
// --------------------------------------------------------
#endif // _wsio_fwd_h_