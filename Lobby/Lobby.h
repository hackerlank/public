//
//  Lobby.h
//  Lobby
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Lobby_h
#define Lobby_h

class Lobby :public keye::ws_service {
public:
                    Lobby(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    virtual void	on_http(const http_parser& req,http_parser& resp);
    MsgHandler      handler;

    static Lobby*   sLobby;
    std::shared_ptr<vic_proxy>   spdb;
};

#endif /* Lobby_h */
