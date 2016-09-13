//
//  DoudeZhu.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
#include <algorithm>
using namespace proto3;

int DoudeZhu::Type(){
    return pb_enum::RULE_DDZ;
}

int DoudeZhu::MaxPlayer(){
    return 3;
}

bool DoudeZhu::Ready(Game& game){
    return game.ready>=MaxPlayer();
}

void DoudeZhu::Deal(Game& game){
    //clear
    game.token=game.banker;
    game.units.clear();
    game.pile.clear();
    game.gameData.resize(MaxPlayer());
    for(auto& gd:game.gameData)gd.clear();
    //init cards
    size_t N=54;
    game.units.resize(N);
    game.pile.resize(N);
    unit_id_t id=0;
    for(int i=3;i<=15;++i){ //A-14, 2-15
        for(int j=0;j<4;++j){
            game.pile[id]=id;
            auto& u=game.units[id];
            u.set_color(j);
            u.set_value(i);
            u.set_id(id++);
        }
    }
    for(int j=0;j<=1;++j){  //Joker-16,17
        game.pile[id]=id;
        auto& u=game.units[id];
        u.set_color(j);
        u.set_value(16+j);
        u.set_id(id++);
    }
    
    //shuffle
    std::random_shuffle(game.pile.begin(),game.pile.end());
    std::random_shuffle(game.pile.begin(),game.pile.end());
    if(rand()>RAND_MAX/2)
        std::random_shuffle(game.pile.begin(),game.pile.end());
    
    //deal
    size_t I=game.banker,
    J=(game.banker+1)%MaxPlayer(),
    K=(game.banker+2)%MaxPlayer();
    std::copy(game.pile.begin(),    game.pile.begin()+20,   std::back_inserter(game.gameData[I].deck));
    std::copy(game.pile.begin()+20, game.pile.begin()+37,   std::back_inserter(game.gameData[J].deck));
    std::copy(game.pile.begin()+37, game.pile.end(),        std::back_inserter(game.gameData[K].deck));
    
    //broadcast
    MsgNCStart msg;
    msg.set_mid(pb_msg::MSG_NC_START);
    msg.set_banker(game.banker);
    auto cards=msg.mutable_cards();
    for(int i=0;i<N;++i){
        auto card=cards->Add();
        card->CopyFrom(game.units[i]);
    }
    int M=1;//MaxPlayer();
    for(int i=0;i<M;++i){
        auto p=game.players[i];
        msg.set_pos(i);
        auto hands=msg.mutable_hands();
        auto n=(int)game.gameData[i].deck.size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.gameData[i].deck[j]);
        p->send(msg);
        hands->Clear();
    }
}

void DoudeZhu::OnDiscard(Player& player,proto3::MsgCNDiscard& msg){
    //MsgNCDiscard omsg;
    //omsg.set_mid(eMsg::MSG_NC_DISCARD);
}

void DoudeZhu::Settle(Game& game){
    
}

bool DoudeZhu::IsGameOver(Game& game){
    return false;
}
