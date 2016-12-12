//
//  Server.h
//  Server
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Server_h
#define Server_h

class Server :public keye::ws_service {
public:
                    Server(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    void            run(const char* =nullptr);
    virtual void	on_http(const http_parser& req,http_parser& resp);
    MsgHandler      handler;
    void            setup_log(const char*);
    int             quantity(float money);
    void            order(const proto3::MsgCPOrder&,proto3::MsgPCOrder&);

    static Server*   sServer;
    std::shared_ptr<vic_proxy>   spdb;
};

#endif /* Server_h */
