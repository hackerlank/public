//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "robot_fwd.h"
using namespace proto3;

void MsgHandler::on_read(keye::svc_handler& sh, void* buf, size_t sz){
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    switch (mid) {
        case eMsg::MSG_NC_ENTER:{
            MsgNCEnter imsg;
            if(pb.Parse(imsg)){
                KEYE_LOG("----entered node\n");
                MsgCNCreate omsg;
                omsg.set_mid(MSG_CN_CREATE);
                omsg.set_rule(proto3::pb_enum::RULE_GENERIC);
                omsg.set_category(proto3::pb_enum::CATEGORY_EASY);
                omsg.set_robot(2);
                PBHelper::Send(sh,omsg);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
            }
            break;
        }
        case eMsg::MSG_NC_CREATE:{
            MsgNCCreate imsg;
            if(pb.Parse(imsg)){
                auto gid=imsg.game_id();
                KEYE_LOG("----game created id=%d\n",gid);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
            }
            break;
        }
        default:
            break;
    }
    KEYE_LOG("----on_read %zd,mid=%d\n", sz,mid);
}

void MsgHandler::on_response(const http_parser& resp) {
    auto msgid=resp.header("msgid");
    auto body=resp.body();
    auto mid=(eMsg)atoi(msgid);
    switch(mid){
        case eMsg::MSG_SC_LOGIN:{
            auto str=base64_decode(body);
            proto3::MsgSCLogin imsg;
            if(imsg.ParseFromString(str)){
                KEYE_LOG("----login succeeded\n");
                auto ip=imsg.ip();
                auto port=imsg.port();
                char uri[64];
                sprintf(uri,"http://%s:%d",ip.c_str(),port);
                robot::sRobot->http.set_uri(uri);
                robot::sRobot->http.enter_lobby();
            }else{
                KEYE_LOG("----login error\n");
            }
            break;
        }
        case eMsg::MSG_LC_ENTER:{
            auto str=base64_decode(body);
            proto3::MsgLCEnter imsg;
            if(imsg.ParseFromString(str)){
                KEYE_LOG("----enter lobby succeeded\n");
                const char* host="127.0.0.1";
                unsigned short port = 8820;
                robot::sRobot->node.connect(host,port);
            }else{
                KEYE_LOG("----enter lobby error\n");
            }
            break;
        }
        default:
            break;
    }
}
