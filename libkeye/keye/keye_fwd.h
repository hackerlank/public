// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: core_fwd.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-30
 */
// --------------------------------------------------------
#ifndef _core_fwd_h_
#define _core_fwd_h_
// --------------------------------------------------------
#include <keye/utility/utility_fwd.h>
#include <keye/htio/htio_fwd.h>
#include <keye/keyeio/keyeio_fwd.h>
#include "keye/mysql_proxy/mysql_proxy_fwd.h"
#ifdef _USE_LIBWEBSOCKET_
#include <keye/wsio/wsio_fwd.h>
#endif
#ifdef _USE_WEBSOCKETPP_
#include "keye/wsio/wsio_fwd.h"
#endif
// --------------------------------------------------------
#endif // _core_fwd_h_
