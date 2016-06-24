// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: logger.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#ifndef _logger_h_
#define _logger_h_

#pragma warning(disable:4251)	//avoid export implementation class

namespace keye{
// --------------------------------------------------------
// logger
// --------------------------------------------------------
class logger_impl;
class KEYE_API logger{
public:
				logger(const char* =nullptr);
	logger&		operator<<(char v);
	logger&		operator<<(int v);
	logger&		operator<<(unsigned int v);
	logger&		operator<<(long long v);
	logger&		operator<<(unsigned long long v);
	logger&		operator<<(const char* v);
	logger&		operator<<(const std::string& v);
	logger&		operator<<(logger& (*)(logger&));
	void		flush();
private:
	s_ptr<logger_impl>	_impl;
};

KEYE_API logger& begl(logger&);		//begin line with time stamp[MM-DD-HH:MM:SS]
KEYE_API logger& endl(logger&);		//change line
KEYE_API logger& endf(logger&);		//change line and flush
KEYE_API logger& flush(logger&);	//flush log
// --------------------------------------------------------
};// namespace
#endif
