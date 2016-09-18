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
    for(auto& gd:game.gameData)gd.Clear();
    //init cards
    size_t N=54;
    game.units.resize(N);
    game.pile.resize(N);
    unit_id_t id=0;
    for(int i=1;i<=13;++i){ //A-K => 1-13
        for(int j=0;j<4;++j){
            game.pile[id]=id;
            auto& u=game.units[id];
            u.set_color(j); //clubs,diamonds,hearts,spades => 0-3
            u.set_value(i);
            u.set_id(id++);
        }
    }
    for(int j=0;j<=1;++j){  //Joker(color 0,1) => 14
        game.pile[id]=id;
        auto& u=game.units[id];
        u.set_color(j);
        u.set_value(14);
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
    /*
    std::copy(game.pile.begin(),    game.pile.begin()+20,   std::back_inserter(game.gameData[I].hands()));
    std::copy(game.pile.begin()+20, game.pile.begin()+37,   std::back_inserter(game.gameData[J].hands()));
    std::copy(game.pile.begin()+37, game.pile.end(),        std::back_inserter(game.gameData[K].hands()));
    */
    for(auto x=game.pile.begin(),       xx=game.pile.begin()+20;    x!=xx;++x)game.gameData[I].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20,    xx=game.pile.begin()+20+17; x!=xx;++x)game.gameData[J].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20+17, xx=game.pile.end();         x!=xx;++x)game.gameData[K].mutable_hands()->Add(*x);
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
    for(int i=0;i<MaxPlayer();++i)
        msg.mutable_count()->Add((int)game.gameData[i].hands().size());
    auto bankerHands=game.gameData[I].hands().size();
    for(auto i=bankerHands-3;i<bankerHands;++i)
        msg.mutable_bottom()->Add(game.gameData[I].hands(i));
    
    int M=1;//MaxPlayer();
    for(int i=0;i<M;++i){
        auto p=game.players[i];
        msg.set_pos(i);
        auto hands=msg.mutable_hands();
        auto n=(int)game.gameData[i].hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.gameData[i].hands(j));
        
        p->send(msg);
        hands->Clear();
    }
}

void DoudeZhu::OnDiscard(Player& player,proto3::MsgCNDiscard& msg){
    if(auto game=player.game){
        if(game->token==player.pos){
            //MsgNCDiscard omsg;
            //omsg.set_mid(eMsg::MSG_NC_DISCARD);
        }else
            KEYE_LOG("OnDiscard wrong pos %d(need %d)\n",player.pos,game->token);
    }else
        KEYE_LOG("OnDiscard no game\n");
}

void DoudeZhu::Settle(Game& game){
    
}

bool DoudeZhu::IsGameOver(Game& game){
    return false;
}
