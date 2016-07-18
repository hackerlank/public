// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_parameters.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_parameters_h_
#define _mysql_parameters_h_

namespace keye{
	class parameter_impl;
};
template class KEYE_API std::shared_ptr<keye::parameter_impl>;

namespace keye{
// --------------------------------------------------------
// mysql parameter
// --------------------------------------------------------
class KEYE_API mysql_parameter{
public:
				mysql_parameter(size_t num,size_t id=0,void* privdata=nullptr);
				mysql_parameter(const void*);
	void		push_byte(char);
	void		push_short(short);
	void		push_int(int);
	void		push_longlong(long long);
	void		push_string(const char*);
	void		push_blob(const void*,size_t);
	//parameter number
	size_t		size()const;
	//id for result handler,pass to mysql_resultset while execute
	size_t		id()const;
	void		id(size_t);
	//from buffer
	void		serialize(const void*);
	//to buffer
	s_ptr<char>	deserialize(size_t&)const;
private:
	friend class mysql_statement;
	friend class mysql_statement_impl;
	s_ptr<parameter_impl>	_impl;
};
// --------------------------------------------------------
};// namespace
#endif // mysql_resultset_h_
