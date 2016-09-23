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

void GameRule::Tick(Game& game){
    switch (game.state) {
        case Game::State::ST_WAIT:
            if(Ready(game))
                ChangeState(game,Game::State::ST_START);
            break;
        case Game::State::ST_START:
            Deal(game);
            ChangeState(game,Game::State::ST_DISCARD);
            break;
        case Game::State::ST_DISCARD:
            if(IsGameOver(game))
                ChangeState(game,Game::State::ST_SETTLE);
            break;
        case Game::State::ST_MELD:
            break;
        case Game::State::ST_SETTLE:
            if(Settle(game))
                ChangeState(game,Game::State::ST_END);
            else
                ChangeState(game,Game::State::ST_WAIT);
            break;
        case Game::State::ST_END:
            break;
        default:
            break;
    }
    PostTick(game);
}

void GameRule::PostTick(Game& game){
}

void GameRule::Deal(Game& game){
    //clear
    game.token=game.banker;
    game.units.clear();
    game.pile.clear();
    game.historical.clear();
    game.gameData.resize(MaxPlayer());
    for(auto& gd:game.gameData)gd.Clear();
    
    //init cards
    auto N=MaxCards();
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
    auto MH=MaxHands();
    auto MM=MH+Bottom();
    size_t I=game.banker,
    J=(game.banker+1)%MP,
    K=(game.banker+2)%MP;
    bunch_t bottom;
    auto sorter=std::bind(&GameRule::comparision,this,game,std::placeholders::_1,std::placeholders::_2);
    for(auto x=game.pile.begin()+MH,    xx=game.pile.begin()+MM;    x!=xx;++x)bottom.add_pawns(*x);
    std::sort(game.pile.begin(),           game.pile.begin()+MM,    sorter);
    std::sort(game.pile.begin()+MM,        game.pile.begin()+MM+MH, sorter);
    std::sort(game.pile.begin()+MM+MH,     game.pile.end(),         sorter);
    
    for(auto x=game.pile.begin(),       xx=game.pile.begin()+MM;    x!=xx;++x)game.gameData[I].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+MM,    xx=game.pile.begin()+MM+MH; x!=xx;++x)game.gameData[J].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+MM+MH, xx=game.pile.end();         x!=xx;++x)game.gameData[K].mutable_hands()->Add(*x);

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
        msg.mutable_count()->Add((int)game.gameData[i].hands().size());
    msg.mutable_bottom()->CopyFrom(bottom.pawns());
    
    for(auto p:game.players){
        msg.set_pos(p->pos);
        auto hands=msg.mutable_hands();
        auto n=(int)game.gameData[p->pos].hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.gameData[p->pos].hands(j));
        
        p->send(msg);
        hands->Clear();
    }
}

void GameRule::OnReady(Player& player){
    if(auto game=player.game){
        if(player.isRobot&&game->ready>=MaxPlayer()-1)return;
        
        ++game->ready;
        MsgNCReady omsg;
        omsg.set_mid(pb_msg::MSG_NC_READY);
        omsg.set_pos(player.pos);
        omsg.set_result(pb_enum::SUCCEESS);
        for(auto& p:game->players)p->send(omsg);
    }
}

void GameRule::Next(Game& game){
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
