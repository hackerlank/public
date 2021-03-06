#ifndef _htio_pch_h_
#define _htio_pch_h_
// --------------------------------------------------------
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include <memory>
#include "bas/bas_pch.hpp"
#include "bas/bas_fwd.hpp"
#ifdef _USE_WEBSOCKETPP_
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#endif
// --------------------------------------------------------
#endif // _htio_pch_h_
