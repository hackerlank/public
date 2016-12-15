//
//  MsgHandler.hpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef MsgHandler_hpp
#define MsgHandler_hpp

class MsgHandler{
public:
    void    on_http(const http_parser& req,const std::function<void(const http_parser&)> func);
};

#endif /* MsgHandler_hpp */
