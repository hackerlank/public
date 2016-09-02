//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "LoginFwd.h"
using namespace proto3;

void MsgHandler::on_http(const http_parser& req,http_parser& resp){
    auto msgid=req.header("msgid");
    auto body=req.body();
    if(atoi(msgid)==eMsg::MSG_CS_LOGIN){
        auto str=base64_decode(body);
        proto3::MsgCSLogin imsg;
        proto3::MsgSCLogin omsg;
        auto mid=eMsg::MSG_SC_LOGIN;
        omsg.set_mid(mid);
        if(imsg.ParseFromString(str)){
            KEYE_LOG("----client login succeeded\n");
            omsg.set_uid("clusters");
            omsg.set_version(imsg.version()+1);
            omsg.set_ip("127.0.0.1");
            omsg.set_port(8810);
            omsg.set_result(proto3::pb_enum::SUCCEESS);
            
            PBHelper::Response(resp,omsg,mid);
        }else{
            KEYE_LOG("----client login failed\n");
            PBHelper::Response(resp,omsg,mid,500,"Internal error");
        }
    }
}
