// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: mysql_connection.hpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-29
 */
// --------------------------------------------------------
#ifndef _mysql_connection_h_
#define _mysql_connection_h_

namespace keye{
// --------------------------------------------------------
// mysql_connection
// --------------------------------------------------------
class mysql_connection{
public:
					~mysql_connection(){close();}
	s_ptr<mysql_resultset> execute(mysql_statement& stmt,mysql_parameter* params){
						s_ptr<mysql_resultset> res;
						return stmt._impl?stmt._impl->execute(mysql_,params):res;
					}
	void			close(){
						if (mysql_ ){
							mysql_close(mysql_);
							mysql_ = NULL;
						}
					}
protected:
	friend class mysql_proxy_impl;
					mysql_connection(MYSQL* mysql):mysql_(mysql){}
	MYSQL*			mysql_;
};
// --------------------------------------------------------
};// namespace
#endif // _mysql_connection_h_
