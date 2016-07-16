#ifndef _proxy_fwd_h_
#define _proxy_fwd_h_
// --------------------------------------------------------
#ifndef s_ptr
#	define s_ptr std::shared_ptr
#endif

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