//
//  Login.h
//  Login
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Login_h
#define Login_h

class Login :public keye::ws_service {
public:
                    Login(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    virtual void	on_http(const http_parser& req,http_parser& resp);
    MsgHandler      handler;

    static Login*   sLogin;
};

#endif /* Login_h */
