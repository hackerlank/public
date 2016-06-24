#ifndef _robot_ai_h_
#define _robot_ai_h_

class RobotAi{
public:
					RobotAi(Robot& r):robot(r){}
	virtual			~RobotAi(){}
	virtual void	action()=0;
protected:
	Robot&			robot;
};

class AiWrite:public RobotAi{
public:
					AiWrite(Robot& r):RobotAi(r){};
};

class AiMove:public RobotAi{
public:
					AiMove(Robot& r,int x,int y)
						:RobotAi(r),ipos(0),_x(x),_y(y){};
	virtual void	action();
private:
	size_t			ipos;
	int				_x,_y;
};

class AiChangeScene:public RobotAi{
public:
					AiChangeScene(Robot& r):RobotAi(r),curr(-1),last(-1){};
	void			push(size_t);
	virtual void	action();
private:
	size_t				curr,last;
	std::vector<size_t>	scenes;
};

#endif