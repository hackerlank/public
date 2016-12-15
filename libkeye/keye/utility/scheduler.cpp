// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: scheduler.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2011-4-24
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <keye/utility/utility_fwd.h>
#include "threadpool/threadpool.hpp"

using namespace keye;
using namespace std;
// ----------------------------------------------------------------
static void _taskFunc(itask* task){
	if(task)task->run();
}
// --------------------------------------------------------
namespace keye{
class scheduler_impl{
public:
							scheduler_impl(unsigned short num=1);
	virtual void			resize(unsigned short);
	//schedule a task and run once
	virtual void			schedule(itask*);
    virtual void			schedule(const task_func&);
	//wait all tasks exit
	virtual void			join();
private:
	std::shared_ptr<boost::threadpool::pool>	_pool;
};};

scheduler_impl::scheduler_impl(unsigned short num){
	resize(num);
}

void scheduler_impl::resize(unsigned short n){
	if(_pool)	_pool->size_controller().resize(n);
	else		_pool.reset(new boost::threadpool::pool(n));
}

void scheduler_impl::schedule(itask* task){
	_pool->schedule(std::bind(_taskFunc,task));
}

void scheduler_impl::schedule(const task_func& func){
    _pool->schedule(func);
}

void scheduler_impl::join(){
	_pool->wait();
}
// --------------------------------------------------------
scheduler::scheduler(unsigned short num){
	_impl.reset(new scheduler_impl(num));
}

void scheduler::resize(unsigned short n){
	if(_impl)_impl->resize(n);
}

void scheduler::schedule(itask* t){
	if(_impl)_impl->schedule(t);
}

void scheduler::schedule(const task_func& func){
    if(_impl)_impl->schedule(func);
}

void scheduler::join(){
	if(_impl)_impl->join();
}
