// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_connection.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_statement_h_
#define _mysql_statement_h_

namespace keye{
class mysql_statement_impl;
template class KEYE_API std::shared_ptr<mysql_statement_impl>;
// procedure and statement compatible
class KEYE_API mysql_statement{
public:
						/*name must be procdure name or sql statement;
							while db is null,using default database*/
						mysql_statement(const char* name,const char* db=nullptr);
	const char*			name()const;
	const char*			database()const;
private:
	friend class mysql_connection;
	s_ptr<mysql_statement_impl>	_impl;
};
// --------------------------------------------------------
};// namespace
#endif // mysql_resultset_h_
