//
//  robot.h
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef robot_h
#define robot_h

class robot :public keye::ws_client{
public:
                    robot();
    virtual void	on_open(keye::svc_handler& sh);
    virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
    virtual bool	on_timer(keye::svc_handler& sh, size_t id, size_t milliseconds);
    
    MsgHandler      handler;
private:
    char _buf[WRITE_MAX];
};

#endif /* robot_h */
