#ifndef _proxy_pch_h_
#define _proxy_pch_h_
// --------------------------------------------------------
#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <vector>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <mutex>

#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif
// mysql
#include <mysql.h>
// --------------------------------------------------------
#endif // _proxy_pch_h_