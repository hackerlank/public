//
//  Lobby.h
//  Lobby
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Lobby_h
#define Lobby_h

class Lobby :public Server {
public:
                    Lobby();
    virtual bool    run(const char* =nullptr);
    virtual void	on_http(const http_parser& req,http_parser& resp,const std::function<void(const http_parser&)> func);
    MsgHandler      handler;

    static Lobby*   sLobby;
};

#endif /* Lobby_h */
