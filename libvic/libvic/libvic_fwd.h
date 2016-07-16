// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: libvic_fwd.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#ifndef _libvic_fwd_h_
#define _libvic_fwd_h_
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

#include "libvic/vic_proxy.h"
#include "libvic/redis_proxy/redis_proxy_fwd.h"
#include "libvic/mysql_proxy/mysql_proxy_fwd.h"
// --------------------------------------------------------
#endif // _libvic_fwd_h_
