//
//  Player.cpp
//  Node
//
//  Created by Vic Liu on 9/7/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "ImmortalFwd.h"
using namespace proto3;

Player::Player(keye::svc_handler& sh)
:ready(false)
,engaged(false)
,inputCount(0)
,lastHand(0){
    spsh=sh();
    playData.set_seat(-1);
}

void Player::on_read(PBHelper& pb){
    if(!spsh)return;
    auto& sh=*spsh;
    auto mid=pb.Id();
    
    if(mid>pb_msg::MSG_CN_JOIN &&(!game || !game->rule)){
        Debug<<"no game when handle "<<(int)mid<<endl;
        return;
    }
    
    switch (mid) {
        case proto3::pb_msg::MSG_CN_CREATE:{
            MsgCNCreate imsg;
            const auto omid=proto3::pb_msg::MSG_NC_CREATE;
            if(!pb.Parse(imsg)){
                MsgNCCreate omsg;
                omsg.set_result(proto3::pb_enum::ERR_PROTOCOL);
                omsg.set_mid(omid);
                PBHelper::Send(sh,omsg);
                break;
            }
            
            Immortal::sImmortal->tpool.schedule(std::bind([](
                                                             decltype(spsh) Spsh,
                                                             Player* player,
                                                             MsgCNCreate imsg){
                MsgNCCreate omsg;
                omsg.set_mid(omid);
                do{
                    //gold check
                    auto spdb=Immortal::sImmortal->spdb;
                    auto uid=player->playData.player().uid().c_str();
                    spdb->lock(uid);
                    {
                        char gold_key[128];
                        std::string str;
                        int gold=0;
                        auto cost=(int)Immortal::sImmortal->config.value("goldcost");
                        sprintf(gold_key,"player:%s",uid);
                        spdb->hget(gold_key,"gold",str);
                        gold=atoi(str.c_str());
                        if(gold<cost){
                            omsg.set_result(proto3::pb_enum::ERR_NOENOUGH);
                            spdb->unlock(uid);
                            Debug<<"game create failed,no gold for "<<imsg.game()<<":"<<gold<<endl;
                            break;
                        }
                    }
                    spdb->unlock(uid);
                    
                    auto key=player->getKey();
                    auto gameptr=Immortal::sImmortal->createGame(key,imsg);
                    if(!gameptr){
                        omsg.set_result(proto3::pb_enum::ERR_PARAM);
                        Debug<<"game create failed,no rule "<<imsg.game()<<endl;
                        break;
                    }
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
                    
                    player->game=gameptr;
                    gameptr->players.push_back(player->shared_from_this());
                    gameptr->Round=maxRound;
                    //game->banker=game->rule->MaxPlayer(*game)-1; //test change banker
                    player->ready=true;
                    player->playData.set_seat((int)gameptr->players.size()-1);
                    //fill data
                    
                    omsg.set_game_id((int)gameptr->id);
                    omsg.set_result(proto3::pb_enum::SUCCEESS);
                    //Debug<<"game created,gid="<<(int)game->id<<endl;
                    
                }while(false);
                
                PBHelper::Send(*Spsh,omsg);
            },spsh, this, imsg));
            
            break;
        }
        case proto3::pb_msg::MSG_CN_JOIN:{
            MsgCNJoin imsg;
            MsgNCJoin omsg;
            if(pb.Parse(imsg)){
                auto gid=imsg.game_id();
                if(auto gameptr=Immortal::sImmortal->findGame(gid)){
                    auto rule=gameptr->rule;
                    if(!rule->Ready(*gameptr)){
                        game=gameptr;
                        game->players.push_back(shared_from_this());
                        ready=true;
                        playData.set_seat((int)game->players.size()-1);
                        omsg.set_result(proto3::pb_enum::SUCCEESS);
                        //Debug<<"game joined,gid="<<gid<<endl;
                    }else{
                        omsg.set_result(proto3::pb_enum::ERR_FAILED);
                        Debug<<"game join failed of full,gid="<<gid<<endl;
                    }
                }else{
                    omsg.set_result(proto3::pb_enum::ERR_FAILED);
                    Debug<<"game join failed of no,gid="<<gid<<endl;
                }
            }else{
                Debug<<"game join failed of message error id="<<mid<<endl;
                omsg.set_result(proto3::pb_enum::ERR_PROTOCOL);
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
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
            PBHelper::Send(sh,omsg);
            break;
        }

        case proto3::pb_msg::MSG_CN_REVIVE:{
            MsgCNRevive imsg;
            MsgNCRevive msg;
            msg.set_mid(pb_msg::MSG_NC_REVIVE);
            msg.set_result(pb_enum::ERR_FAILED);

            std::shared_ptr<Player> spPlayer=shared_from_this();
            while (pb.Parse(imsg)){
                auto gameId=imsg.game();
                auto spGame=Immortal::sImmortal->findGame(gameId);
                if(!spGame){
                    Debug<<"reconnect: game "<<gameId<<" not found"<<endl;
                    break;
                }else{
                    //handle player
                    auto uid=playData.player().uid();
                    auto found=false;
                    for(auto p:spGame->players){
                        if(p->playData.player().uid()==uid){
                            found=true;
                            
                            //we donnot copy player, just swap sh
                            auto newsh=spsh;
                            spsh=p->spsh;
                            //close current player
                            Immortal::sImmortal->on_close(*spsh);
                            Immortal::sImmortal->on_close(*newsh);
                            
                            //revive player network handler
                            p->spsh=newsh;
                            Immortal::sImmortal->addPlayer(newsh->id(),p);
                            spPlayer=p;
                            break;
                        }
                    }
                    if(!found){
                        Debug<<"reconnect: game "<<gameId<<" player not found"<<endl;
                        break;
                    }
                    
                    //handle play data
                    auto MP=spGame->rule->MaxPlayer(*spGame);
                    auto& start=*msg.mutable_deal();
                    start.set_banker(spGame->banker);
                    start.set_pos(spPlayer->playData.seat());
                    start.set_ante(spGame->anti);
                    start.set_multiple(spGame->multiple);
                    for(int i=0;i<MP;++i)
                        start.mutable_count()->Add((int)spGame->players[i]->playData.hands().size());
                    for(auto b:spGame->bottom)start.add_bottom(b);
                    start.set_piles((int)spGame->pile.size());
                    
                    //copy data for all
                    for(int i=0,ii=MP;i<ii;++i){
                        auto p=spGame->players[i];
                        auto msgplay=msg.add_play();
                        //seat,select_card,bunch,discards,achvs
                        msgplay->set_seat(p->playData.seat());
                        msgplay->set_selected_card(p->playData.selected_card());
                        msgplay->mutable_bunch()->CopyFrom(p->playData.bunch());
                        
                        //AAAAs if necessary
                        for(auto& aaaa:p->AAAAs)
                            msgplay->mutable_bunch()->Add()->CopyFrom(aaaa);
                        
                        msgplay->mutable_discards()->CopyFrom(p->playData.discards());
                        msgplay->mutable_achvs()->CopyFrom(p->playData.achvs());
                    }
                    
                    //hand cards
                    auto hands=start.mutable_hands();
                    hands->CopyFrom(spPlayer->playData.hands());
                    //AAAs
                    for(auto& aaa:spPlayer->AAAs)
                        for(auto a:aaa.pawns())
                            hands->Add(a);
                    
                    msg.set_result(pb_enum::SUCCEESS);
                    
                    break;
                }
            }

            spPlayer->send(msg);
            if(msg.result()==pb_enum::SUCCEESS && spPlayer->lastMsg){
                //send last message
                spPlayer->send(*spPlayer->lastMsg);
                Debug<<"send last message\n";
            }

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
    //Debug<<"on_read %zd,mid=%d\n", sz,mid);
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
    play_t play(playData);
    playData.Clear();
    playData.set_seat(play.seat());
    playData.mutable_player()->CopyFrom(play.player());
    
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
