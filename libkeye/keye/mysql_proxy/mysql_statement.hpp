// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_statement.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_statement_hpp_
#define _mysql_statement_hpp_

namespace keye{
class mysql_statement_impl{
public:
						/*name must be procdure name or sql statement;
							while db is null,using default database*/
						mysql_statement_impl(const char* name,const char* db=nullptr);
	s_ptr<mysql_resultset>	execute(MYSQL*,mysql_parameter*);
	const char*			name() const	{return _name.empty()?nullptr:_name.c_str();}
	const char*			database()const	{return _db.empty()?nullptr:_db.c_str();}
	const char*			sql() const		{return _sql.empty()?nullptr:_sql.c_str();}
protected:
	friend class mysql_connection;
	s_ptr<mysql_statement_impl>	_impl;
	void					_prepare(const mysql_parameter*);

	std::string			_name,_db,_sql;
};
// --------------------------------------------------------
};// namespace

#endif // mysql_resultset_hpp_
