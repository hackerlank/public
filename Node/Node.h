//
//  slogin.h
//  slogin
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef slogin_h
#define slogin_h

class slogin :public keye::ws_service {
public:
                    slogin(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    virtual void	on_open(keye::svc_handler&);
    virtual void	on_close(keye::svc_handler&);
    virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
    virtual void	on_write(keye::svc_handler&, void*, size_t sz);
    virtual bool	on_timer(keye::svc_handler&, size_t id, size_t milliseconds);
    
    MsgHandler      handler;
};

#endif /* slogin_h */
