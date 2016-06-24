#include "stdafx.h"
#include <keye/cache/cache_fwd.h>
#include <keye/mysql_proxy/mysql_proxy_fwd.h>
#include <keye/utility/utility_fwd.h>

using namespace keye;
using namespace std;

typedef keye_allocator<std_allocator> Scalar_Alloc;

void test_cache();
void test_shm_cache();

class provider;
class city_provider;
class city_handler;
typedef city_provider Provider;
typedef city_handler Handler;

void cache_test(){
	test_cache();
//	test_shm_cache();
}
// --------------------------------------------------------
// helpers
// --------------------------------------------------------
static void tee(const void* buf){
	if(buf)LOG("tee: \"%s\"\n",(char*)buf);
	else LOG("tee: null\n");
}

class Cache_alloc:public cache_alloc{
public:
	Cache_alloc(shm_allocator& sx)
	  :_ax(1*1024*1024,sx){}
	virtual void*	allocate(size_t _Count){return _ax.allocate(_Count);}
	virtual void	deallocate(void* _Ptr, size_t=0){_ax.deallocate(_Ptr);}
private:
	typedef keye_allocator<shm_allocator> shm_keye;
	shm_keye		_ax;
};
// --------------------------------------------------------
// simple data provider
// --------------------------------------------------------
class provider:public cache_provider{
public:
	provider(cache_handler* =nullptr){
		const size_t len=64,sz=sizeof(cache_data)+len;
		char* ptr=new char[sz];
		_buf.reset(ptr,std::default_delete<char[]>());
		_data=(cache_data*)ptr;
		_data->length=len;
		pop(0);
	}
	//push into cache
	void	push(cache_key key,const void* str,size_t){
		if(str)
			strcpy((char*)_data->data,(const char*)str);
	};
	//pop from cache
	void	pop(cache_key key){
		strcpy((char*)_data->data,"empty");
	}
	//access cache
	void	access(cache_key key,cache_handler* hx=nullptr){
		LOG("provider access ==>\n");
		if(hx)hx->handle(key,nullptr,0);
	}
	void	make_line(std::string& line,cache_key k,const void* buf,size_t sz){
		line.resize(sz);
		memcpy((void*)line.data(),buf,sz);
	}
private:
	s_ptr<char>	_buf;
	cache_data*	_data;
};

class Cache_handler:public cache_handler{
public:
	virtual void	handle(cache_key k,const void* buf,size_t){tee(buf);}
};
// --------------------------------------------------------
// mysql proxy provider
// --------------------------------------------------------
class city_provider;
class city_bridge:public mysql_handler{
//bridge handler between mysql provider and external cache handler
public:
						city_bridge(cache_handler* hx):_hx(hx){}
	virtual void		handle(mysql_resultset& rs);
private:
	cache_handler*		_hx;
};

class city_handler:public cache_handler{
public:
	virtual void	handle(cache_key k,const void*,size_t);
};

class city_provider:public cache_provider{
	friend class city_bridge;
public:
	city_provider(cache_handler* hx=nullptr):proxy(1),bridge(hx),_hx(hx){
		//connnect
		const char
			*host="127.0.0.1",
			*user="root",
			*pswd="keye",
			*db="world";
		unsigned short port=3306;
		proxy.connect(host,port,user,pswd,db);
	}
	//push into cache
	void	push(cache_key key,const void* str,size_t len){
		if(str){
			//parse data into mysql_resultset
			mysql_resultset mrs(str,len);
			if(!mrs.rows.empty()){
				auto& row=mrs.rows[0];
				if(row.fields.size()>=2){
					auto buf=(const char*)row.fields[1];
					unsigned short buf_len=0;
					buf_get(buf_len,&buf);
					sprintf(sql,"insert into city value(%d,\'%s\',\'0\',\'0\',0)",key,(const char*)buf);
					//call mysql proxy
					stmt.reset(new mysql_statement(sql));
					proxy.execute(*stmt.get(),nullptr);
				}
			}
		}
	};
	//pop from cache
	void	pop(cache_key key){
		sprintf(sql,"delete from city where ID=%d",key);
		stmt.reset(new mysql_statement(sql));
		//call mysql proxy
		proxy.execute(*stmt.get(),nullptr);
	}
	//access cache
	void	access(cache_key key,cache_handler* =nullptr){
		sprintf(sql,"select * from city where ID=%d",key);
		mysql_parameter pm(0,key);
		mysql_statement st(sql);
		//call mysql proxy and pass into bridge
		proxy.execute(st,&pm,&bridge);
	}
	void	make_line(std::string& line,cache_key k,const void* buf,size_t sz){
		/*-----+------+-------------+----------+------------+
		| ID   | Name | CountryCode | District | Population |
		+------+------+-------------+----------+-----------*/
		mysql_resultset mrs(nullptr,0);
		mysql_resultset_builder bu(mrs);
		bu.add_field(EF_INT);
		bu.add_field(EF_STRING);
		bu.add_field(EF_STRING);
		bu.add_field(EF_STRING);
		bu.add_field(EF_INT);
		row_t row;
		row.length=5*sizeof(int)+sz;
		size_t zero=0;
		std::shared_ptr<char> spdata(new char[sz+sizeof(int)],std::default_delete<char[]>());
		auto pdata=spdata.get();
		row.fields.push_back(&k);		//ID
		row.fields.push_back(pdata);	//Name
		row.fields.push_back(&zero);	//CountryCode
		row.fields.push_back(&zero);	//District
		row.fields.push_back(&zero);	//Population
		buf_set(&pdata,sz);
		buf_set(&pdata,buf,sz);
		bu.add_row(row);
		bu.build(k);

		line.resize(mrs.length);
		memcpy((void*)line.data(),mrs.buffer.get(),mrs.length);
		int x=0;
	}
private:
	mysql_proxy proxy;
	city_bridge bridge;
	cache_handler* _hx;
	std::shared_ptr<mysql_statement>	stmt;
	std::shared_ptr<mysql_parameter>	stpm;
	char sql[256];
};

void city_bridge::handle(mysql_resultset& rs){
	if(_hx){
		//bridge of mysql,call external cache_handler(city_handler)
		_hx->handle(rs.id,rs.buffer.get(),rs.length);
	}
}

void city_handler::handle(cache_key k,const void* buf,size_t len){
	//handle final mysql resultset
	mysql_resultset mrs(buf,len);
}
// --------------------------------------------------------
// test routine
// --------------------------------------------------------
void test_cache(){
	cache_key key=4088;
	const char* str="jiangyou";
	size_t len=strlen(str);

	Handler hx;
	Provider px(&hx);
	px.access(key);			//access provider

	shm_allocator shmax("jonyo",2*1024*1024);
	Cache_alloc keye_alloc(shmax);

	cache<Cache_alloc,Handler,Provider> db_cache(keye_alloc,&hx,&px);

	db_cache.access(key);	//access provider and cache

	std::string line;
	px.make_line(line,key,str,len);
	db_cache.push(key,line.c_str(),line.length());
//	db_cache.access(key);	//access cache
	db_cache.flush();
	db_cache.access(key);	//access cache

	db_cache.access(key);	//access cache
	db_cache.pop(key);
	db_cache.access(key);	//access cache
	db_cache.flush();
	db_cache.access(key);	//access provider and cache

	keye::pause();
}
// --------------------------------------------------------
void test_shm_cache(){
	cache_key key=4088;
	const char* str="jiangyou";
	size_t len=strlen(str);

	Handler hx;
	Provider px(&hx);
	px.access(key);		//access provider

	shm_cache<Handler,Provider> db_cache("jonyo",2*1024*1024,&hx,&px);

	db_cache.access(key);	//access provider and cache
	std::string line;
	px.make_line(line,key,str,len);
	db_cache.push(key,line.c_str(),len);
	db_cache.push(key,str,len);
	db_cache.access(key);	//access cache
	keye::pause();
	db_cache.pop(key);
	db_cache.access(key);	//access provider and cache
	db_cache.flush();
	db_cache.access(key);	//access provider and cache

	keye::pause();
}
