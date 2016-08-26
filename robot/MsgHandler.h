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
    void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
};

#endif /* MsgHandler_hpp */
