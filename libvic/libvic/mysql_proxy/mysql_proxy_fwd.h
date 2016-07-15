#ifndef _proxy_fwd_h_
#define _proxy_fwd_h_
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

//#include <libvic/utility/utility_fwd.h>
#include <libvic/mysql_proxy/mysql_resultset.h>
#include <libvic/mysql_proxy/mysql_parameters.h>
#include <libvic/mysql_proxy/mysql_statement.h>
#include <libvic/mysql_proxy/mysql_proxy.h>
#include <libvic/mysql_proxy/buf_set.h>

#ifdef _DEBUG
#define LINFO KEYE_LOG
#else
#define LINFO
#endif
// --------------------------------------------------------
#endif // _proxy_fwd_h_