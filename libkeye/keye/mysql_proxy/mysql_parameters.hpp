// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_parameters.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_parameters_hpp_
#define _mysql_parameters_hpp_

namespace keye{
// --------------------------------------------------------
// mysql parameter implement
// --------------------------------------------------------
class parameter_impl{
public:
				parameter_impl(size_t num,size_t id=0,void* privdata=nullptr);
				parameter_impl(const void*);
	bool		prepare(MYSQL*);
	size_t		size()const;
	size_t		id()const;
	void		id(size_t);
	void		serialize(const void*);
	s_ptr<char>	deserialize(size_t&)const;
	void*		privdata;
private:
	friend class mysql_statement_impl;
	friend class mysql_parameter;
	s_ptr<MYSQL_BIND>			_mysql_binds;		//all MYSQL_BIND
	std::vector<s_ptr<char>>	_mysql_binds_bufs;	//all MYSQL_BIND.buffer
	unsigned char		_index,_num;				//parameter numbers and index
	unsigned short		_length;					//
	size_t				_id;

	void		_push_param_v(const void* val,e_field t,size_t len=0);
	template<typename T>
	void		_push_param(T val,e_field t,size_t len=0){
					if(0==len)len=sizeof(T);
					_push_param_v((const void*)&val,t,len);
				}
};
// --------------------------------------------------------
};
#endif // mysql_resultset_hpp_
