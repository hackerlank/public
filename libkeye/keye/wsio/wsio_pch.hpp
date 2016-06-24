#ifndef _wsio_pch_h_
#define _wsio_pch_h_
// --------------------------------------------------------
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#ifdef _USE_LIBWEBSOCKET_
#include <libwebsocket/libwebsockets.h>
#endif

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
// --------------------------------------------------------
#endif // _wsio_pch_h_