#ifndef _stdafx_h_
#define _stdafx_h_

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <vector>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <string.h>
#include <functional>
#include <time.h>

#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

#define _USE_WEBSOCKETPP_
#include <keye/keye_fwd.h>
#include <libvic/libvic_fwd.h>

#endif // _stdafx_h_