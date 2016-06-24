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
#ifndef _htio_fwd_h_
#define _htio_fwd_h_
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

#include <keye/htio/svc_handler.h>
#include <keye/htio/metric.h>
#include <keye/htio/packer.h>
#include <keye/htio/op_packer.h>
#include <keye/htio/service.h>
#include <keye/htio/http_client.h>
// --------------------------------------------------------
#endif // _htio_fwd_h_