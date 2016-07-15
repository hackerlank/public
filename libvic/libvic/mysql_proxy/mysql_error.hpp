// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_error.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_error_h_
#define _mysql_error_h_

namespace keye{
	void throw_mysql_error(const char* api, const char* error_message);
	inline void throw_mysql_error(const char* api, const char* error_message)
	{
		std::string message;
		message = api;
		message += "() failed, ";
		message += error_message;
		throw std::runtime_error(message);
	}

	void throw_stmt_error(const char* api, MYSQL_STMT* stmt, const char* sql);
	inline void throw_stmt_error(const char* api, MYSQL_STMT* stmt, const char* sql)
	{
		unsigned int error_code = mysql_stmt_errno(stmt);
		const char* error_message = mysql_stmt_error(stmt);
		std::string message;
		message = api;
		message += "() failed, err(";
		message += error_code;
		message += "): ";
		message += error_message;
		message += " sql: ";
		message += sql;
		throw std::runtime_error(message);
	}
// --------------------------------------------------------
};// namespace
#endif // _mysql_error_h_
