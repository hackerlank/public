#ifndef _utility_fwd_h_
#define _utility_fwd_h_
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

#include <keye/utility/system.h>
#include <keye/utility/logger.h>
#include <keye/utility/buf_set.h>
#include <keye/utility/conf_file.h>
#include <keye/utility/str_util.h>
#include <keye/utility/md5.h>
#include <keye/utility/secure_hash.h>
// --------------------------------------------------------
#endif // _utility_fwd_h_