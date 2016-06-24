// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_resultset.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_resultset_h_
#define _mysql_resultset_h_

namespace keye{
class mysql_resultset_impl;
// --------------------------------------------------------
enum e_field:unsigned char{
	EF_BYTE,
	EF_SHORT,
	EF_INT,
	EF_LONGLONG,
	EF_STRING,
	EF_BLOB
};

struct KEYE_API row_t{
	size_t				length;
	std::vector<void*>	fields;
};
// --------------------------------------------------------
class KEYE_API mysql_resultset{
public:
	//accessable data
	std::vector<e_field>	head;	//fields type
	std::vector<row_t>		rows;	//data pointers
	size_t					length;	//data length
	s_ptr<char>				buffer;	//data buffer
	size_t					id;		//id for result handler,pass by mysql_parameter while execute
	std::string				error;	//error message if happen
	void					debug_log();
	void*					privdata;
	//ctor from data buffer
	mysql_resultset(const void* =nullptr,size_t len =0,void* privdata=nullptr);
private:
	friend class mysql_statement_impl;
	friend class mysql_resultset_builder;
	s_ptr<mysql_resultset_impl>	_impl;
};
// --------------------------------------------------------
class KEYE_API mysql_resultset_builder{
public:
				mysql_resultset_builder(mysql_resultset&);
	void		add_field(e_field);
	void		add_row(row_t&);
	bool		build(size_t id);
private:
	mysql_resultset&	_rs;
};
// --------------------------------------------------------
};// namespace
#endif // _mysql_row_h_
