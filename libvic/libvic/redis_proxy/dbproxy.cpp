// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: db_proxy.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2011-4-22
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <libvic/libvic_fwd.h>

namespace keye{
class db_proxy_impl{
public:
};
};

using namespace keye;

// --------------------------------------------------------
// db_proxy
// --------------------------------------------------------
db_proxy::db_proxy(unsigned char conns){
}

bool db_proxy::connect(const char* host, unsigned short port,
		const char* user, const char* passwd,const char* db){
	return true;
}
