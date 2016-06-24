#ifndef _ba_pch_h_
#define _ba_pch_h_
// --------------------------------------------------------
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

// boost
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <vector>
#include <memory>
#include <map>
#include <thread>

#define _LOG
#ifdef _LOG
#	define KEYE_LOG printf
#else
#	define KEYE_LOG
#endif
// --------------------------------------------------------
#endif // _ba_pch_h_