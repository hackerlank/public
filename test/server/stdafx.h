#ifndef _stdafx_h_
#define _stdafx_h_

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <vector>
#include <memory>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <string.h>

#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <keye/keye_fwd.h>
#include "alloc_test.h"
#include "cache_test.h"
#include "mysql_test.h"
#include "utility_test.h"
#include "htio_fx.h"

#endif // _stdafx_h_