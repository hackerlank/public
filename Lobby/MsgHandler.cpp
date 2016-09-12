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

void MsgHandler::on_http(const http_parser& req,http_parser& resp){
    auto msgid=req.header("msgid");
    auto body=req.body();
    auto mid=(pb_msg)atoi(msgid);
    switch(mid){
        case pb_msg::MSG_CL_ENTER:{
            auto str=base64_decode(body);
            proto3::MsgCLEnter imsg;
            proto3::MsgLCEnter omsg;
            auto omid=pb_msg::MSG_LC_ENTER;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                KEYE_LOG("----client enter succeeded\n");
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                KEYE_LOG("----client enter failed\n");
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
        }
        default:
            break;
    }
}
