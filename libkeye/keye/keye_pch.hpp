// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: core_pch.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-30
 */
// --------------------------------------------------------
#ifndef _core_pch_h_
#define _core_pch_h_

#define _USE_WEBSOCKETPP_
//#define _USE_LIBWEBSOCKET_
/* This defined the latest Window platform.
	If you are compiling for the preversion of Windows platform,Please include <WinSDKVer.h>
	and define proper WIN32_WINNT,then include <SDKDDKVer.h>*/
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include "keye/utility/utility_pch.hpp"
#include "keye/htio/htio_pch.hpp"
#include "keye/keyeio/keyeio_pch.hpp"
#include "keye/mysql_proxy/mysql_proxy_pch.hpp"
#ifdef _USE_LIBWEBSOCKET_
#include "keye/wsio/wsio_pch.hpp"
#endif
// --------------------------------------------------------
#endif // _core_pch_h_
