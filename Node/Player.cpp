//
//  Player.cpp
//  Node
//
//  Created by Vic Liu on 9/7/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

Player::Player(keye::svc_handler& sh){
    spsh=sh();
}

void Player::on_read(PBHelper& pb){
    if(!spsh)return;
    auto& sh=*spsh;
    auto mid=pb.Id();
    switch (mid) {
        case eMsg::MSG_CN_CREATE:{
            MsgCNCreate imsg;
            MsgNCCreate omsg;
            if(pb.Parse(imsg)){
                if(auto rule=Node::sNode->findGame(imsg.rule())){
                    auto deskptr=Node::sNode->createGame(imsg);
                    rule->Create(*this,deskptr);
                    omsg.set_game_id(deskptr->id);
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    KEYE_LOG("----game created,gid=%zd\n",deskptr->id);
                }else{
                    omsg.set_result(proto3::pb_enum::ERR_FAILED);
                    KEYE_LOG("----game create failed,no rule %d\n",imsg.rule());
                }
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_NC_CREATE);
            PBHelper::Send(sh,omsg);
            break;
        }
        case eMsg::MSG_CN_JOIN:{
            MsgCNJoin imsg;
            MsgNCJoin omsg;
            if(pb.Parse(imsg)){
                auto gid=imsg.game_id();
                if(auto rule=Node::sNode->findGame(proto3::RULE_DDZ)){
                    Desk desk;
                    rule->Join(desk,*this);
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    KEYE_LOG("----game joined,gid=%d\n",gid);
                }else{
                    omsg.set_result(proto3::pb_enum::ERR_FAILED);
                    KEYE_LOG("----game join failed,gid=%d\n",gid);
                }
            }else{
                KEYE_LOG("----message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(eMsg::MSG_NC_JOIN);
            PBHelper::Send(sh,omsg);
            break;
        }
        default:
            break;
    }
    //KEYE_LOG("----on_read %zd,mid=%d\n", sz,mid);
}
