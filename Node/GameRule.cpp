//
//  GameRule.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"
#include <random>
#include <algorithm>
using namespace proto3;

void GameRule::deal(Game& game){
    //clear
    game.token=game.banker;
    game.units.clear();
    game.pile.clear();
    game.historical.clear();
    game.pendingMeld.clear();
    game.pendingDiscard.reset();
    for(auto& player:game.players)if(player)player->reset();
    
    //init cards
    auto N=maxCards();
    game.units.resize(N);
    game.pile.resize(N);
    initCard(game);
    
    //shuffle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(game.pile.begin(),game.pile.end(),gen);
    std::shuffle(game.pile.begin(),game.pile.end(),gen);
    
    //deal: fixed position,movable banker
    auto MP=MaxPlayer();
    auto MH=maxHands();
    auto BK=MH+bottom();
    size_t I=game.banker,
    J=(game.banker+1)%MP,
    K=(game.banker+2)%MP;
    bunch_t bottom;
    auto sorter=std::bind(&GameRule::comparision,this,game,std::placeholders::_1,std::placeholders::_2);
    for(auto x=game.pile.begin()+MH,    xx=game.pile.begin()+BK;    x!=xx;++x)bottom.add_pawns(*x);
    std::sort(game.pile.begin(),           game.pile.begin()+BK,    sorter);
    std::sort(game.pile.begin()+BK,        game.pile.begin()+BK+MH, sorter);
    std::sort(game.pile.begin()+BK+MH,     game.pile.end(),         sorter);
    
    for(auto x=game.pile.begin(),       xx=game.pile.begin()+BK;      x!=xx;++x)game.players[I]->gameData.mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+BK,    xx=game.pile.begin()+BK+MH*1; x!=xx;++x)game.players[J]->gameData.mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+BK+MH, xx=game.pile.begin()+BK+MH*2; x!=xx;++x)game.players[K]->gameData.mutable_hands()->Add(*x);
    
    game.pile.erase(game.pile.begin(),game.pile.begin()+BK+MH*2);
    for(auto x=game.pile.begin(),xx=game.pile.end();x!=xx;++x)game.pileMap[*x]=0;    

    //broadcast
    MsgNCStart msg;
    msg.set_mid(pb_msg::MSG_NC_START);
    msg.set_banker(game.banker);
    msg.set_ante(10);
    msg.set_multiple(1);
    auto cards=msg.mutable_cards();
    for(int i=0;i<N;++i){
        auto card=cards->Add();
        card->CopyFrom(game.units[i]);
    }
    for(int i=0;i<MP;++i)
        msg.mutable_count()->Add((int)game.players[i]->gameData.hands().size());
    msg.mutable_bottom()->CopyFrom(bottom.pawns());
    
    for(auto p:game.players){
        msg.set_pos(p->pos);
        auto hands=msg.mutable_hands();
        auto n=(int)game.players[p->pos]->gameData.hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.players[p->pos]->gameData.hands(j));
        
        p->send(msg);
        hands->Clear();
    }
    
    //first discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(game.token);
}

void GameRule::OnReady(Player& player){
    if(auto game=player.game){
        if(player.ready)return;
        
        player.ready=true;
        MsgNCReady omsg;
        omsg.set_mid(pb_msg::MSG_NC_READY);
        omsg.set_pos(player.pos);
        omsg.set_result(pb_enum::SUCCEESS);
        for(auto& p:game->players)p->send(omsg);
    }
}

bool GameRule::Ready(Game& game){
    int n=0;
    for(auto p:game.players)if(p&&p->ready)++n;
    return n>=MaxPlayer();
}

void GameRule::next(Game& game){
    auto old=game.token;
    if(++game.token>=game.rule->MaxPlayer())game.token=0;
    KEYE_LOG("game token: %d=>%d\n",old,game.token);
}

void GameRule::ChangeState(Game& game,Game::State state){
    if(game.state!=state){
        KEYE_LOG("game state: %d=>%d\n",game.state,state);
        game.state=state;
    }
}

void GameRule::logHands(Game& game,uint32 pos,std::string msg){
    std::string str;
    auto& hands=game.players[pos]->gameData.hands();
    cards2str(game,str,hands);
    KEYE_LOG("%s hand of %d:%d %s\n",msg.c_str(),pos,hands.size(),str.c_str());
}

const char* GameRule::bunch2str(Game& game,std::string& str,const proto3::bunch_t& bunch){
    char buf[32];
    sprintf(buf,"ops=%d",(int)bunch.type());
    cards2str(game,str,bunch.pawns());
    str=buf+str;
    return str.c_str();
}

const char* GameRule::cards2str(Game& game,std::string& str,const google::protobuf::RepeatedField<uint32>& ids){
    str.clear();
    char buf[32];
    for(auto id:ids){
        sprintf(buf,"(%d:%d),",id,game.units[id].value());
        str+=buf;
    }
    return str.c_str();
}
