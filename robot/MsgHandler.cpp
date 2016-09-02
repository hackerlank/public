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
        case eMsg::MSG_SC_LOGIN:{
            MsgSCLogin imsg;
            if(pb.Parse(imsg)){
                auto ip=imsg.ip();
                auto port=imsg.port();
                robot::sRobot->lobby.connect(ip.c_str(),port);
                KEYE_LOG("----enter lobby host=%s,port=%d\n",ip.c_str(),port);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
            }
            break;
        }
        case eMsg::MSG_LC_ENTER:{
            MsgLCEnter imsg;
            if(pb.Parse(imsg)){
                const char* host="127.0.0.1";
                unsigned short port = 8820;
                robot::sRobot->node.connect(host,port);
                KEYE_LOG("----enter node host=%s,port=%d\n",host,port);
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
            }
            break;
        }
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
    int mid=0;
    if(atoi(msgid)==eMsg::MSG_SC_LOGIN){
        auto str=base64_decode(body);
        proto3::MsgSCLogin imsg;
        if(imsg.ParseFromString(str)){
            mid=imsg.mid();
        }
    }
    KEYE_LOG("----on_response mid=%d\n",mid);
}
