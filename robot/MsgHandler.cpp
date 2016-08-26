//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "robot_fwd.h"

void MsgHandler::on_read(keye::svc_handler& sh, void* buf, size_t sz){
    KEYE_LOG("----on_read %zd\n", sz);
    
    KEYE_LOG("read %d:%s\n", (int)sz, (char*)buf);
}
