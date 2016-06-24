#include "stdafx.h"
using namespace keye;
// --------------------------------------------------------
class myalloc:public htio_alloc{
public:
	virtual void*	allocate(size_t _Count){return _sax.allocate(_Count);}
	virtual void	deallocate(void* _Ptr, size_t){_sax.deallocate((char*)_Ptr,0);}
private:
typedef std::allocator<char> std_ax_t;
	std_ax_t		_sax;
};
// --------------------------------------------------------
class s_handler:public work_handler{
public:
	virtual void	on_read(svc_handler& sh,void* buf,size_t sz){
		const char* request="<policy-file-request/>";
		const char* response="<cross-domain-policy><site-control permitted-cross-domain-policies=\"all\"/><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>";
		const size_t sz_request=22,
			sz_response=141;
		if(buf&&sz>=sz_request){
			char str[sz_request+1];
			memcpy(str,buf,sz_request);
			str[sz_request]='\0';
			if(!strcmp(request,str)){
				std::string ip(sh.address());
				LOG("client %s permitted.\n",ip.c_str());
				sh.send((void*)response,sz_response);
			}
		}
	}
};
// --------------------------------------------------------
class B{
public:
	typedef void(*other)();
	void f(void* o){
		((other)o)();
	}
};
template<typename T>
class A{
public:
//#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address)-(unsigned long)(&((type*)0)->field)))
	void f(T& t){
//		void* addr=((void*)((char*)(&t)-(unsigned long)(&T::run)));
//		b.f(addr);
	}
	B b;
};
class C{
public:
	void run(){
		printf("concreate.\n");
	}
};
int main(int argc,char* argv[]){
	s_handler w;
	myalloc a;
	size_t rb_size=32;
	typedef service<work_handler,myalloc> service_t;
	service_t s(w,a,1,1,rb_size);
	s.run(843);

	C cc;
	A<C> aa;
	aa.f(cc);

	LOG("press x or ctrl+c to exit.\n");
	bool exit=false;
	while(!exit)
	switch(std::getchar()){
	case 'x':
		exit=true;
		break;
	}
	keye::pause();
	return 0;
}
