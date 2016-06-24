#include "stdafx.h"
#include <keye/allocator/alloc_fwd.h>
#include <keye/utility/utility_fwd.h>

using namespace keye;
using namespace std;

typedef keye_allocator<std_allocator> Scalar_Alloc;

void test_time();
void test_mthreadd_alloc();
void test_shm();
void test_grow();
void test_mutex();
void test_shrink();

void alloc_test(){
//	test_shrink();
	test_mthreadd_alloc();
//	test_time();
//	test_shm();
}

void test_shrink(){
	size_t TIMES=10000;
	std_allocator std_alloc;
	Scalar_Alloc scalar_alloc(8*1024,std_alloc);
	std::vector<void*> ptrs;
	for(size_t i=0;i<TIMES;++i){
		auto p=scalar_alloc.allocate(4*1024-20);
		ptrs.push_back(p);
	}
	scalar_alloc.shrink(1);
	for(auto i=ptrs.rbegin();i!=ptrs.rend();++i){
		if(auto p=*i)
			scalar_alloc.deallocate(p);
	}
	scalar_alloc.shrink(1);
	keye::pause();
}

void test_shm(){
//	char buf[1024];

	shm_allocator shm_alloc("jonyc",0x20000,(const void*)0x3000000);
	void* ptr=shm_alloc.allocate(1024);
	shm_alloc.deallocate(ptr);

	shm_allocator shm_alloc1("jonyd",0x20000,(const void*)0x3010000);
	ptr=shm_alloc1.allocate(1024);
	shm_alloc1.deallocate(ptr);
}

template<typename _Alloc>
void svc(_Alloc& alloc,const char* desc,size_t TIMES){
	time_t t0,t1;
	t0=ticker();
	for(size_t j=0;j<TIMES;++j)
		for(int i=MIN_ORDER_BIT;i<MAX_ORDER_BIT;++i)
			if(char* p=(char*)alloc.allocate(1<<i)){
				*p='a';
				alloc.deallocate((typename _Alloc::value_type*)p,0);
			}else
				LOG("error allocate by %s.\n",desc);
	t1=ticker();
	LOG("times: %d,%s: %d\n",TIMES*(MAX_ORDER_BIT-MIN_ORDER_BIT),desc,t1-t0);
}

void test_time(){
	size_t TIMES=10000;
//	size_t t0,t1,t2,t3,t4,t5;

	std_allocator std_alloc;
	Scalar_Alloc scalar_alloc(128*1024*1024,std_alloc);
	LOG("total:  %d,alloc: %d\n",scalar_alloc.max_size(),scalar_alloc.size());

	svc<std_allocator>(std_alloc,"std_alloc",TIMES);
	svc<Scalar_Alloc>(scalar_alloc,"scalar_alloc",TIMES);
}

void test_mthreadd_alloc(){
	size_t TIMES=10000;
//	size_t t0,t1,t2,t3,t4,t5;

	std_allocator std_alloc;
	typedef copyable_allocator<Scalar_Alloc> cax;
	Scalar_Alloc scalar_alloc_base(128*1024*1024,std_alloc);
	LOG("total:  %d,alloc: %d\n",scalar_alloc_base.max_size(),scalar_alloc_base.size());
	cax keye_alloc(scalar_alloc_base);
/*
	using namespace boost::interprocess;
	boost::thread th_std(boost::bind(svc<std_allocator>,std_alloc,"std_alloc",TIMES));
	boost::thread th_keye(boost::bind(svc<cax>,keye_alloc,"keye_alloc",TIMES));
	boost::thread th_std0(boost::bind(svc<std_allocator>,std_alloc,"std_alloc",TIMES));
	boost::thread th_keye0(boost::bind(svc<cax>,keye_alloc,"keye_alloc",TIMES));
	boost::thread th_std1(boost::bind(svc<std_allocator>,std_alloc,"std_alloc",TIMES));
	boost::thread th_keye1(boost::bind(svc<cax>,keye_alloc,"keye_alloc",TIMES));
	boost::thread th_std2(boost::bind(svc<std_allocator>,std_alloc,"std_alloc",TIMES));
	boost::thread th_keye2(boost::bind(svc<cax>,keye_alloc,"keye_alloc",TIMES));
*/
	keye::pause();
/*	test result
	times: 80000,scalar_alloc: 244
	times: 80000,scalar_alloc: 309
	times: 80000,scalar_alloc: 332
	times: 80000,scalar_alloc: 362
	times: 80000,keye_alloc: 299
	times: 80000,keye_alloc: 344
	times: 80000,keye_alloc: 350
	times: 80000,keye_alloc: 360
	times: 80000,std_alloc: 701
	times: 80000,std_alloc: 729
	times: 80000,std_alloc: 761
	times: 80000,std_alloc: 784
*/
}
