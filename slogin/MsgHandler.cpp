//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "slogin_fwd.h"
using namespace proto3;

void MsgHandler::on_read(keye::svc_handler& sh, void* buf, size_t sz){
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    switch (mid) {
        case eMsg::MSG_CS_LOGIN:{
            MsgCSLogin imsg;
            MsgSCLogin omsg;
            if(pb.Parse(imsg)){
                omsg.set_uid("clusters");
                omsg.set_version(imsg.version()+1);
                omsg.set_ip("127.0.0.1");
                omsg.set_port(8900);
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_SC_LOGIN);
            PBHelper::Send(sh,omsg);
            break;
        }
        default:
            break;
    }
    KEYE_LOG("----on_read %zd,mid=%d\n", sz,mid);
}
