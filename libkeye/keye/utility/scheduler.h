// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: scheduler.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2011-4-24
 */
// --------------------------------------------------------
#ifndef _scheduler_h_
#define _scheduler_h_

namespace keye{
    
    typedef std::function<void()> task_func;

// --------------------------------------------------------
// itask
// --------------------------------------------------------
class KEYE_API itask{
public:
	virtual					~itask(){};
	//thread body
	virtual void			run()=0;
	virtual int				exit()=0;
};
// --------------------------------------------------------
// scheduler
// --------------------------------------------------------
class scheduler_impl;
class KEYE_API scheduler{
public:
							scheduler(unsigned short num=1);
	virtual void			resize(unsigned short);
	//schedule a task and run once
    virtual void			schedule(const task_func&);
	virtual void			schedule(itask*);
	//wait all tasks exit
	virtual void			join();
private:
	std::shared_ptr<scheduler_impl>	_impl;
};
// --------------------------------------------------------
};// namespace
#endif // _scheduler_h_
