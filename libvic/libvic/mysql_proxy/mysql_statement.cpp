// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: Receiver.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2011-4-22
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <libvic/libvic_fwd.h>
#include <libvic/mysql_proxy/mysql_parameters.hpp>
#include <libvic/mysql_proxy/mysql_statement.hpp>
#include <libvic/mysql_proxy/mysql_resultset.hpp>
#include <errmsg.h>

using namespace keye;

mysql_statement_impl::mysql_statement_impl(const char* name,const char* db)
:_name(name){
	if(db)_db=db;
}

void mysql_statement_impl::_prepare(const mysql_parameter* params){
	if(_sql.empty()&&!_name.empty()){
		if(std::string::npos==_name.find(' ')){
			//procedure
			_sql = "CALL "+_name+"(";
			if(params)
			for (size_t i=0,ii=params->size();i<ii;i++){
				if (i)_sql += ",";
				_sql += "?";
			}
			_sql += ")";
		}else{
			//normal query statement
			_sql=_name;
		}
	}
}

s_ptr<mysql_resultset> mysql_statement_impl::execute(MYSQL* mysql,mysql_parameter* params){
	_prepare(params);

	size_t id=(params?params->id():0);
	void* privdata=(params&&params->_impl?params->_impl->privdata:nullptr);
	s_ptr<mysql_resultset> sprs(new mysql_resultset(nullptr,0,privdata));
	if(mysql&&!_sql.empty()){
		try{
			//mysql_stmt_init
			if(MYSQL_STMT* stmt = mysql_stmt_init(mysql)){
				try{
					if(mysql_stmt_prepare(stmt,_sql.c_str(),(unsigned long)_sql.length()))
						throw std::runtime_error("mysql_stmt_prepare error.");
					if(params&&params->_impl){
						MYSQL_BIND* bind_params=params->_impl->_mysql_binds.get();
						unsigned long number_of_params=(unsigned long)params->size();
						if(bind_params != NULL && number_of_params != 0){
							if(mysql_stmt_param_count(stmt) != number_of_params)
								throw std::invalid_argument("invalid parameter count");
							if(mysql_stmt_bind_param(stmt,bind_params))
								throw std::runtime_error("mysql_stmt_bind_param error.");
						}

						LINFO("execute \"%s\"\n",_sql.c_str());
						//size_t len;
						//params->deserialize(len);
					}

					if(mysql_stmt_execute(stmt)){
						//try execute from mysql console
						throw std::runtime_error(mysql_stmt_error(stmt));
					}
					//result set
					sprs->_impl.reset(new mysql_resultset_impl(*sprs,stmt,_sql.c_str(),id));
				}catch(std::exception& err){
					sprs->_impl.reset(new mysql_resultset_impl(*sprs,nullptr,_sql.c_str(),id));
					sprs->error=err.what();
					KEYE_LOG("%s\n",err.what());
				}
				//free result
				int status = 0;
				do{
					// call procedures returns multiple result set
					//(the continueous result set were empty but need been taken, or causes OUT_OF_SYNC)
					status = mysql_stmt_next_result(stmt);
				} while(status == 0);
				mysql_stmt_free_result(stmt);
			}else
				throw std::runtime_error("mysql_stmt_init error.");
		}catch(std::exception& err){
			sprs->_impl.reset(new mysql_resultset_impl(*sprs,nullptr,_sql.c_str(),id));
			sprs->error=err.what();
			KEYE_LOG("%s",err.what());
		}
	}
	return sprs;
}
// --------------------------------------------------------
mysql_statement::mysql_statement(const char* name,const char* db){
	_impl.reset(new mysql_statement_impl(name,db));
}

const char* mysql_statement::name()const{return _impl?_impl->name():nullptr;}
const char* mysql_statement::database()const{return _impl?_impl->database():nullptr;}
