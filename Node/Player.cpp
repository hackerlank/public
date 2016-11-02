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

Player::Player(keye::svc_handler& sh)
:pos(-1)
,ready(false)
,engaged(false)
,inputCount(0)
,lastHand(0){
    spsh=sh();
}

void Player::on_read(PBHelper& pb){
    if(!spsh)return;
    auto& sh=*spsh;
    auto mid=pb.Id();
    switch (mid) {
        case proto3::pb_msg::MSG_CN_CREATE:{
            MsgCNCreate imsg;
            MsgNCCreate omsg;
            if(pb.Parse(imsg)){
                auto key=getKey();
                auto gameptr=Node::sNode->createGame(key,imsg);
                if(gameptr){
                    int maxRound=1;
                    for(auto kv:imsg.options()){
                        switch (kv.ikey()) {
                            case pb_enum::OPTION_CATEGORY:
                                gameptr->category=(pb_enum)kv.ivalue();
                                break;
                            case pb_enum::OPTION_ROUND:
                                maxRound=kv.ivalue();
                                break;
                            case pb_enum::OPTION_DEFINED_CARDS:
                                gameptr->definedCards=kv.value();
                                break;
                            default:
                                break;
                        }
                    }

                    game=gameptr;
                    game->players.push_back(shared_from_this());
                    game->Round=maxRound;
                    ready=true;
                    pos=game->players.size()-1;
                    //fill data
                
                    omsg.set_game_id((int)game->id);
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    KEYE_LOG("game created,gid=%zd\n",game->id);
                }else{
                    omsg.set_result(proto3::pb_enum::ERR_FAILED);
                    KEYE_LOG("game create failed,no rule %d\n",imsg.game());
                }
            }else{
                KEYE_LOG("message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(proto3::pb_msg::MSG_NC_CREATE);
            PBHelper::Send(sh,omsg);
            
            break;
        }
        case proto3::pb_msg::MSG_CN_JOIN:{
            MsgCNJoin imsg;
            MsgNCJoin omsg;
            if(pb.Parse(imsg)){
                auto gid=imsg.game_id();
                if(auto gameptr=Node::sNode->findGame(gid)){
                    auto rule=gameptr->rule;
                    if(!rule->Ready(*gameptr)){
                        game=gameptr;
                        game->players.push_back(shared_from_this());
                        ready=true;
                        pos=game->players.size()-1;
                        omsg.set_result(proto3::pb_enum::SUCCEESS);
                        KEYE_LOG("game joined,gid=%d\n",gid);
                    }else{
                        omsg.set_result(proto3::pb_enum::ERR_FAILED);
                        KEYE_LOG("game join failed of full,gid=%d\n",gid);
                    }
                }else{
                    omsg.set_result(proto3::pb_enum::ERR_FAILED);
                    KEYE_LOG("game join failed of no,gid=%d\n",gid);
                }
            }else{
                KEYE_LOG("game join failed of message error id=%zd\n",mid);
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            omsg.set_mid(proto3::pb_msg::MSG_NC_JOIN);
            PBHelper::Send(sh,omsg);
            break;
        }
        case MSG_CN_DISMISS_SYNC:{
            MsgCNDismissSync imsg;
            MsgNCDismissSync omsg;
            omsg.set_mid(proto3::pb_msg::MSG_NC_DISMISS_SYNC);
            if(pb.Parse(imsg)){
                if(game){
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    for(auto p:game->players){
                        if(p!=shared_from_this()){
                            PBHelper::Send(*p->spsh,omsg);
                        }
                    }
                    break;
                }
            }
            KEYE_LOG("game dismiss failed\n");
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
            PBHelper::Send(sh,omsg);
            break;
        }
        case MSG_CN_DISMISS_ACK:{
            MsgCNDismissAck imsg;
            MsgNCDismissAck omsg;
            omsg.set_mid(proto3::pb_msg::MSG_NC_DISMISS_ACK);
            if(pb.Parse(imsg)){
                if(game){
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    for(auto p:game->players){
                        if(p!=shared_from_this()){
                            PBHelper::Send(*p->spsh,omsg);
                        }
                    }
                    break;
                }
            }
            KEYE_LOG("game dismiss failed\n");
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
            PBHelper::Send(sh,omsg);
            break;
        }
        case MSG_CN_READY:{
            game->rule->OnReady(*this);
            break;
        }
        case proto3::pb_msg::MSG_CN_ENGAGE:{
            MsgCNEngage imsg;
            if(pb.Parse(imsg))
                game->rule->OnEngage(*this,imsg.key());
            break;
        }
        case MSG_CN_DISCARD:{
            MsgCNDiscard imsg;
            if(pb.Parse(imsg))
                game->rule->OnDiscard(*this,imsg);
            break;
        }
        case MSG_CN_MELD:{
            MsgCNMeld imsg;
            if(pb.Parse(imsg))
                game->rule->OnMeld(*this,imsg.bunch());
            break;
        }
        default:
            break;
    }
    //KEYE_LOG("on_read %zd,mid=%d\n", sz,mid);
}

void Player::send(google::protobuf::MessageLite& msg){
    PBHelper::Send(*spsh,msg);
}

int Player::getKey(){
    if(spsh){
        auto uri=spsh->address();
        auto i=uri.rfind("/");
        if(i!=std::string::npos){
            auto r=uri.substr(i+1);
            return atoi(r.c_str());
        }
    }
    return 0;
}

void Player::reset(){
    playData.Clear();
    unpairedCards.clear();
    dodgeCards.clear();
    conflictMeld=false;
    AAAs.clear();
    AAAAs.clear();
    lastMsg.reset();
    inputCount=0;
    lastHand=invalid_card;
    m_winMark=0;
    m_bczArr=false;
    m_bdoubleTi=false;
}
