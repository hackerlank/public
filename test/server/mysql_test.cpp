#include "stdafx.h"
#include <keye/mysql_proxy/mysql_proxy_fwd.h>
#include <keye/utility/utility_fwd.h>

using namespace keye;

class my_handler:public mysql_handler{
public:
	virtual void		handle(mysql_resultset& resultset){
		mysql_resultset rs(resultset.buffer.get(),resultset.length);
	}
};

namespace keye{
void mysql_test(){
	//connnect
	const char
		*host="127.0.0.1",
		*user="root",
		*pswd="keye",
		*db="world";
	unsigned short port=3306;
	mysql_proxy proxy(1);
	my_handler handler;
	proxy.connect(host,port,user,pswd,db);

	mysql_parameter param0(0,23);
	mysql_statement st_find("select * from city limit 0,5");
	auto TIMES=1;
	for(auto i=0;i<TIMES;++i)
		proxy.execute(st_find,&param0,&handler);

	keye::pause();
/*
	//prepare parameters
	mysql_parameter p_insert(4);
	p_insert.push_longlong(1002);
	p_insert.push_int(19);
	p_insert.push_int(0);
	p_insert.push_string("010400820820");
	//prepare procedure
	mysql_statement st_insert("Proc_InsertUser");
	proxy.execute(st_insert,nullptr,&handler);

	//prepare parameters
	mysql_parameter p_find(1);
	p_find.push_byte(19);
	//prepare procedure
	mysql_statement st_find("Proc_FindAge");
	proxy.execute(st_find,nullptr,&handler);
*/
}};
