// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: shm_allocator.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-8-14
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/allocator/alloc_fwd.h>
#ifdef WIN32
#include <Windows.h>
#else
#endif

using namespace keye;
using namespace std;
using namespace boost::interprocess;
// --------------------------------------------------------
// shm_allocator
// --------------------------------------------------------
namespace keye{
class shm_impl{
public:
				shm_impl(const char*,size_t,const void* =0);
				~shm_impl();
	void		initialize(const char*,size_t,const void* =0);
	void*		allocate(size_t _Count);
	void		deallocate(void* _Ptr, size_t=0);
	void*		address()const;
	size_t		max_size()const;	//size total
	size_t		size()const;		//size free
private:
	bool		_initialize(size_t,const void*);
	//must be nocopable
				shm_impl(const shm_impl&);
	shm_impl&	operator=(const shm_impl&);

	std::string	_name;
	s_ptr<managed_shared_memory>	_mshm;
};};
// --------------------------------------------------------
shm_impl::shm_impl(const char* name,size_t sz,const void* addr){
	if(name&&sz){
		shared_memory_object::remove(_name.c_str());	//test only
		_name=name;
		_initialize(sz,addr);
	}
}

shm_impl::~shm_impl(){
	//remove shared memory object
	shared_memory_object::remove(_name.c_str());
}

bool shm_impl::_initialize(size_t sz,const void* addr){
	try{
		//open first
		_mshm.reset(new managed_shared_memory(open_only,_name.c_str(),addr));//open_read_only
		//size adjust
		size_t sz_=_mshm->get_size();
		if(sz>sz_)
			managed_shared_memory::grow(_name.c_str(),sz-sz_);//shrink_to_fit(_name.c_str());
		LOG("managed_shared_memory \"%s\" opened\n",_name.c_str());
	}catch(interprocess_exception& e){
		LOG(e.what());
		try{
			//create if not exists
			_mshm.reset(new managed_shared_memory(create_only,_name.c_str(),sz,addr));//open_or_create
			LOG("managed_shared_memory \"%s\" created\n",_name.c_str());
		}catch(interprocess_exception& e){
			LOG(e.what());
			return false;
		}
	}
	LOG("managed_shared_memory \"%s\":\naddress: 0x%x\nsize: %d\nfree: %d\n",
		_name.c_str(),(size_t)_mshm->get_address(),_mshm->get_size(),_mshm->get_free_memory());
/*
	//start test
	//Construct a named object
	keye_allocator* kalloc=nullptr;
	try{
		if(kalloc=_mshm->find<keye_allocator>("keye_class").first)
			_mshm->destroy<keye_allocator>("keye_class");
		kalloc=_mshm->construct<keye_allocator>("keye_class")();
	}catch(interprocess_exception& e){
		LOG(e.what());
		try{
			kalloc=_mshm->construct<keye_allocator>("keye_class")();
		}catch(interprocess_exception& e){
			LOG(e.what());
		}
	}
	if(kalloc)kalloc->reserve(*this,4096);
	//Map it again
	try{
		managed_shared_memory shm(open_only,_name.c_str());
		//Check "keye_class" is still there
		try{
			if(kalloc=shm.find<keye_allocator>("keye_class").first)
				shm.destroy<keye_allocator>("keye_class");
		}catch(interprocess_exception& e){
			LOG(e.what());
		}
	}catch(interprocess_exception& e){
		LOG(e.what());
	}
	keye::pause();
*/
	return true;
}

void* shm_impl::allocate(size_t _Count){
	void* ptr=nullptr;
	if(_mshm){
		size_t size=_mshm->get_size(),
			szfree=_mshm->get_free_memory();
		if(szfree>=_Count){
			try{
				ptr=_mshm->allocate(_Count);
			}catch(boost::interprocess::bad_alloc& e){
				LOG(e.what());
			}
		}else
			LOG("Out of memory.\n");
	}
	return ptr;
}

void shm_impl::deallocate(void* _Ptr, size_t){
	if(_Ptr&&_mshm){
		try{
			_mshm->deallocate(_Ptr);
		}catch(boost::interprocess::bad_alloc& e){
			LOG(e.what());
		}
	}
}

void* shm_impl::address()const{
	return _mshm?_mshm->get_address():nullptr;
}

size_t shm_impl::max_size()const{
	return _mshm?_mshm->get_size():0;
}

size_t shm_impl::size()const{
	return _mshm?_mshm->get_free_memory():0;
}
// --------------------------------------------------------
shm_allocator::shm_allocator(const char* name,size_t sz,const void* addr){
	_impl.reset(new shm_impl(name,sz,addr));
}
void*	shm_allocator::allocate(size_t _Count)	{return _impl?_impl->allocate(_Count):nullptr;}
void		shm_allocator::deallocate(void* p,size_t){if(_impl)_impl->deallocate(p);}
void*	shm_allocator::address()const			{return _impl?_impl->address():0;}
size_t	shm_allocator::max_size()const			{return _impl?_impl->max_size():0;}
size_t	shm_allocator::size()const				{return _impl?_impl->size():0;}
/*
#include <boost/interprocess/managed_mapped_file.hpp>
void test_mapped_file(){
	const char *ManagedFile = "MyManagedFile"; 
	const char *ManagedFile2 = "MyManagedFile2"; 
	file_mapping::remove(ManagedFile); 
	file_mapping::remove(ManagedFile2); 
	remove_file_on_destroy destroyer(ManagedFile); 
	remove_file_on_destroy destroyer2(ManagedFile2); 

	//Create an named integer in a managed mapped file 
	managed_mapped_file managed_file(create_only, ManagedFile, 65536); 
	managed_file.construct<int>("MyInt")(0u); 
	//Now create a copy on write version 
	managed_mapped_file managed_file_cow(open_copy_on_write, ManagedFile); 
	//Erase the int and create a new one 
	if(!managed_file_cow.destroy<int>("MyInt")) 
		throw int(0); 
	managed_file_cow.construct<int>("MyInt2"); 
	//Check changes 
	if(managed_file_cow.find<int>("MyInt").first && !managed_file_cow.find<int>("MyInt2").first)
		throw int(0); 
	//Check the original is intact 
	if(!managed_file.find<int>("MyInt").first && managed_file.find<int>("MyInt2").first)
		throw int(0); 

	//Dump the modified copy on write segment to a file 
	std::fstream file(ManagedFile2, std::ios_base::out | std::ios_base::binary); 
	if(!file) throw int(0);
	file.write(static_cast<const char *>(managed_file_cow.get_address()), (std::streamsize)managed_file_cow.get_size()); 

	//Now open the modified file and test changes 
	managed_mapped_file managed_file_cow2(open_only, ManagedFile2); 
	if(managed_file_cow2.find<int>("MyInt").first && !managed_file_cow2.find<int>("MyInt2").first) 
		throw int(0); 

	//Now create a read-only version 
	managed_mapped_file managed_file_ro(open_read_only, ManagedFile); 
	//Check the original is intact 
	if(!managed_file_ro.find<int>("MyInt").first && managed_file_ro.find<int>("MyInt2").first) throw int(0); 
	//Check the number of named objects using the iterators 
	if(std::distance(managed_file_ro.named_begin(), managed_file_ro.named_end()) != 1 && std::distance(managed_file_ro.unique_begin(), managed_file_ro.unique_end()) != 0 ) 
		throw int(0); 

	return 0;
}
*/