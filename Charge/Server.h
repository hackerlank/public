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
    virtual void	on_http(const http_parser& req,http_parser& resp);
    MsgHandler      handler;

    static Server*   sServer;
};

#endif /* Server_h */
