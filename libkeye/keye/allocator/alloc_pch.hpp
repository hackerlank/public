// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: alloc_pch.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-3-7
 */
// --------------------------------------------------------
#ifndef _alloc_pch_h_
#define _alloc_pch_h_

/* This defined the latest Window platform.
	If you are compiling for the preversion of Windows platform,Please include <WinSDKVer.h>
	and define proper WIN32_WINNT,then include <SDKDDKVer.h>*/
#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include <map>
#include <vector>

//boost interprocess and thread
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread.hpp>
#define BOOST_THREAD

#ifdef BOOST_THREAD
#	define MUTEX(mtx)		boost::mutex mtx
#	define GUARD_LOCK(mtx)	boost::mutex::scoped_lock lock(mtx)
#	define SMUTEX(mtx)		boost::shared_ptr<boost::mutex> mtx
#	define SCTOR_MUTEX(mtx)	do{mtx.reset(new boost::mutex);}while(0)
#	define SGUARD_LOCK(mtx)	boost::mutex::scoped_lock lock(*(mtx))
	//interprocess_mutex can be placed in shared memory 
	//and can be shared between processes
	typedef boost::interprocess::interprocess_mutex		ip_mutex;
	typedef boost::interprocess::scoped_lock<ip_mutex>	ip_lock;
#	define IPC_MUTEX(mtx)		ip_mutex mtx
#	define IPC_GUARD_LOCK(mtx)	ip_lock lock(mtx)
#else
#	define MUTEX(mtx)
#	define GUARD_LOCK(mtx)
#	define SMUTEX(mtx)
#	define SCTOR_MUTEX(mtx)
#	define SGUARD_LOCK(mtx)
#	define IPC_MUTEX(mtx)
#	define IPC_GUARD_LOCK(mtx)
#endif
// --------------------------------------------------------
#endif // _alloc_pch_h_
