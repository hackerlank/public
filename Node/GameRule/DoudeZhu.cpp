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

bool DoudeZhu::Ready(Desk& desk){
    return desk.ready>=MaxPlayer();
}

void DoudeZhu::Deal(Desk& desk){
    //clear
    desk.token=desk.banker;
    desk.units.clear();
    desk.pile.clear();
    desk.gameData.resize(MaxPlayer());
    for(auto& gd:desk.gameData)gd.clear();
    //init cards
    size_t N=54;
    desk.units.resize(N);
    desk.pile.resize(N);
    unit_id_t id=0;
    for(int i=3;i<=15;++i){ //A-14, 2-15
        for(int j=0;j<4;++j){
            desk.pile[id]=id;
            auto& u=desk.units[id];
            u.color=j;
            u.value=i;
            u.id=id++;
        }
    }
    for(int j=0;j<=1;++j){  //Joker-16,17
        desk.pile[id]=id;
        auto& u=desk.units[id];
        u.color=j;
        u.value=16+j;
        u.id=id++;
    }
    
    //shuffle
    std::random_shuffle(desk.pile.begin(),desk.pile.end());
    std::random_shuffle(desk.pile.begin(),desk.pile.end());
    if(rand()>RAND_MAX/2)
        std::random_shuffle(desk.pile.begin(),desk.pile.end());
    
    //deal
    size_t I=desk.banker,
    J=(desk.banker+1)%MaxPlayer(),
    K=(desk.banker+2)%MaxPlayer();
    std::copy(desk.pile.begin(),    desk.pile.begin()+20,   std::back_inserter(desk.gameData[I].deck));
    std::copy(desk.pile.begin()+20, desk.pile.begin()+37,   std::back_inserter(desk.gameData[J].deck));
    std::copy(desk.pile.begin()+37, desk.pile.end(),        std::back_inserter(desk.gameData[K].deck));
}

void DoudeZhu::Settle(Desk& desk){
    
}

bool DoudeZhu::IsGameOver(Desk& desk){
    return false;
}
