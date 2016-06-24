#ifndef _robot_h_
#define _robot_h_
#include "htio_fx.h"

class RobotAi;
class robot_session;
enum e_action:unsigned char{
	AI_NONE,
	AI_WRITE,
	AI_MOVE,
};

class Robot:public SessionBase<myalloc>{
public:
			Robot(keye::svc_handler&,myalloc&);
	void	login();
	void	action(e_action=AI_NONE);
	virtual void	handle(const keye::packet_t& p){_handle(*(packet_crc_t*)&p);}
	//
	unsigned long long guid;
private:
	void	_handle(packet_crc_t&);

	template<typename> friend class RobotWx;
	std::string		_name;
	e_action		_action;
	std::shared_ptr<RobotAi>			_ai;
};

#endif // _robot_h_