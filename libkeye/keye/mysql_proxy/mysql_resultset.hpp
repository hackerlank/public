// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_resultset.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_resultset_hpp_
#define _mysql_resultset_hpp_

namespace keye{
// --------------------------------------------------------
class mysql_resultset_impl{
public:
				//from buffer
				mysql_resultset_impl(mysql_resultset&,const void* =nullptr,size_t len =0);
private:
	friend class mysql_statement;
	friend class mysql_resultset_builder;
	friend class mysql_statement_impl;
	friend class parameter_impl;
				//from query
				mysql_resultset_impl(mysql_resultset&,MYSQL_STMT*,const char* sql,size_t id=0);
	static size_t			_field_len(e_field);
	static enum_field_types	_map(e_field);
	static e_field			_map(enum_field_types);
};
// --------------------------------------------------------
};// namespace
#endif // _mysql_resultset_hpp_
