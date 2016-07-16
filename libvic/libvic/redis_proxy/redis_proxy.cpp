// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: redis_proxy.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: Vic Liu
 *Date		: 2016-07-15
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <libvic/libvic_fwd.h>

namespace keye{
class redis_proxy_impl{
public:
};
};

using namespace keye;

// --------------------------------------------------------
// redis_proxy
// --------------------------------------------------------
redis_proxy::redis_proxy(unsigned char conns){
}

bool redis_proxy::connect(const char* host, unsigned short port,
		const char* user, const char* passwd,const char* db){
	return true;
}
