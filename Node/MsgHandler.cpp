//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

void MsgHandler::on_read(keye::svc_handler& sh, void* buf, size_t sz){
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    switch (mid) {
        case eMsg::MSG_CN_ENTER:{
            MsgCNEnter imsg;
            MsgNCEnter omsg;
            if(pb.Parse(imsg)){
                omsg.set_result(proto3::pb_enum::SUCCEESS);
                KEYE_LOG("----client entered\n");
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_NC_ENTER);
            PBHelper::Send(sh,omsg);
            break;
        }
        case eMsg::MSG_CN_CREATE:{
            MsgCNCreate imsg;
            MsgNCCreate omsg;
            if(pb.Parse(imsg)){
                omsg.set_result(proto3::pb_enum::SUCCEESS);
                KEYE_LOG("----game created\n");
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_NC_CREATE);
            PBHelper::Send(sh,omsg);
            break;
        }
        default:
            break;
    }
    KEYE_LOG("----on_read %zd,mid=%d\n", sz,mid);
}
