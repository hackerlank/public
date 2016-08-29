//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "LobbyFwd.h"
using namespace proto3;

void MsgHandler::on_read(keye::svc_handler& sh, void* buf, size_t sz){
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    switch (mid) {
        case eMsg::MSG_CL_ENTER:{
            MsgCLEnter imsg;
            MsgLCEnter omsg;
            if(pb.Parse(imsg)){
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_LC_ENTER);
            PBHelper::Send(sh,omsg);
            break;
        }
        default:
            break;
    }
    KEYE_LOG("----on_read %zd,mid=%d\n", sz,mid);
}
