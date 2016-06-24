// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <keye/mysql_proxy/mysql_proxy_fwd.h>
//#include <keye/utility/utility_fwd.h>

#ifdef WIN32
#include <conio.h>
#else
#include <curses.h>
#endif

using namespace keye;
using namespace std;

class my_handler:public mysql_handler{
public:
	virtual void		handle(mysql_resultset& resultset){
		if(resultset.error.empty()){
			resultset.debug_log();
			auto& rs=resultset;
			std::string strFields("fields ");
			for(auto fields:rs.head){
				auto f=(int)fields;
			}
			for(auto& row:rs.rows){
				auto l=row.length;
			}
		}
	}
};

int main(int argc, char* argv[]) {
	/*
	csv_file csv;
	csv.load("char.csv");
	csv.print();
	
	ini_cfg_file cfg;
	cfg.load("DefaultGame.ini");
	auto gatewayHost	=(const char*)cfg.value("GatewayHost");
	auto gatewayPort	=(int)cfg.value("GatewayCPort");
	auto zoneHost		=(const char*)cfg.value("ZoneHost");
	auto zoneCPort		=(int)cfg.value("ZoneCPort");
	auto zoneAPort		=(int)cfg.value("ZoneAPort");
	auto arenaHost		=(const char*)cfg.value("ArenaHost");
	auto arenaPort		=(int)cfg.value("ArenaPort");
	printf("\npress any key to continue ...\n");
	_getch();

	return 0;
	*/
	
	//connnect
	const char
		*host="127.0.0.1",
		*user="root",
		*pswd="mysql",
		*db="world";
	unsigned short port=3306;
	mysql_proxy proxy(1);
	my_handler handler;
	proxy.connect(host,port,user,pswd,db);

	mysql_parameter param0(0,23);

	mysql_statement st_insert("insert into t_users(id,name) values(1000,'test1000');");
	proxy.execute(st_insert,nullptr,&handler);

	mysql_statement st_find("select * from t_users limit 0,5");
	auto TIMES=10;
	for(auto i=0;i<TIMES;++i)
		proxy.execute(st_find,nullptr,&handler);

	mysql_statement st_find_info("select * from t_user_info");
	proxy.execute(st_find_info,nullptr,&handler);

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

	printf("press any key to continue ...\n");
	_getch();

	return 0;
}
