//
//  Mahjong.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

int Mahjong::Type(){
    return pb_enum::GAME_MJ;
}

int Mahjong::MaxPlayer(){
    return 4;
}

int Mahjong::maxCards(){
    return 108;
}

int Mahjong::maxHands(){
    return 13;
}

int Mahjong::bottom(){
    return 1;
}

void Mahjong::initCard(Game& game){
    //id: [color-index-value]
    for(int j=1;j<=3;++j){          //Tong,Suo,Wan => 1-3
        for(int i=1;i<=9;++i){      //1-9
            for(int k=0;k<4;++k){   //xxxx
                unit_id_t id=j*1000+k*100+i;
                game.pile.push_back(id);
            }
        }
    }
}

void Mahjong::engage(Game& game,MsgNCEngage& msg){
    for(auto& p:game.players)msg.set_keys(p->pos,p->playData.selected_card());
}

bool Mahjong::meld(Game& game,Player& player,unit_id_t card,proto3::bunch_t& bunch){
    auto ret=bunch.type();
    auto pos=player.pos;
    if(ret==pb_enum::BUNCH_A){
        //collect after draw
        player.playData.mutable_hands()->Add(card);
        //remove from pile map
        game.pileMap.erase(card);
    }else{
        //erase from hands
        auto& hands=*player.playData.mutable_hands();
        for(auto j:bunch.pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    //KEYE_LOG("OnMeld pos=%d,erase card %d\n",where,*i);
                    hands.erase(i);
                    break;
                }
            }
        }
        //then meld
        auto h=player.playData.add_bunch();
        h->CopyFrom(bunch);
        changePos(game,pos);
    }
    return true;
}

bool Mahjong::isWin(Game& game,Player& player,unit_id_t card,std::vector<proto3::bunch_t>& output){
    auto& hands=player.playData.hands();
    if(hands.size()<2){
        KEYE_LOG("isWin failed: len=%d\n",hands.size());
        return false;
    }
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));
    
    //insert into hand if not in
    if(validId(card)){
        auto inhand=false;
        for(auto i:cards)if(i==card){inhand=true;break;}
        if(!inhand)cards.push_back(card);
    }
    
    auto sorter=std::bind(&Mahjong::comparision,this,std::placeholders::_1,std::placeholders::_2);
    std::sort(cards.begin(),cards.end(),sorter);
    
    auto len=cards.size()-1;
    for(size_t i=0;i!=len;++i){
        auto A=cards[i+0];
        auto B=cards[i+1];
        if(A/1000==B/1000&&A%100==B%100){
            std::vector<unit_id_t> tmp;
            for(size_t j=0;j!=cards.size();++j)if(j!=i&&j!=i+1)tmp.push_back(cards[j]);
            if(isWinWithoutAA(tmp)){
                return true;
            }
        }
    }
    return false;
}

bool Mahjong::isWin(Game&,std::vector<unit_id_t>&,std::vector<proto3::bunch_t>&){
    return true;
}

bool Mahjong::isWinWithoutAA(std::vector<unit_id_t>& cards){
    auto len=cards.size();
    if(len%3!=0)
        return false;
    
    size_t i=0;
    while(i<len){
        //next 3 continuous cards
        auto A=cards[i+0];
        auto B=cards[i+1];
        auto C=cards[i+2];
        
        if(A/1000 == B/1000 && A/1000 == C/1000){
            //same color
            A%=100;B%=100;C%=100;
            
            if((A+1==B && B+1==C) || (A==B && A==C)){
                //great values
                i+=3;
                continue;
            }else if(i+6<=len){
                //next 6 continuous cards
                auto D=cards[i+3];
                auto E=cards[i+4];
                auto F=cards[i+5];
                
                if(D/1000 == E/1000 && D/1000 == F/1000){
                    //same color
                    D%=100;E%=100;F%=100;
                    if(A==B && C==D && E==F && B+1==C && D+1==E){
                        //great values
                        i+=6;
                        continue;
                    }
                }
            }
        }
        //other wise
        return false;
    }
    
    return true;
}

void Mahjong::settle(Player& player,std::vector<proto3::bunch_t>& allSuites,unit_id_t card){
    auto pos=player.pos;
    auto& game=*player.game;
    auto M=MaxPlayer();

    game.spSettle=std::make_shared<MsgNCSettle>();
    auto& msg=*game.spSettle;
    
    for(pos_t i=0;i<M;++i){
        auto play=msg.add_play();
        play->set_win(pos==i&&allSuites.size()>0?1:0);
    }
    
    if(!game.spFinish){
        //prepare final end message
        game.spFinish=std::make_shared<MsgNCFinish>();
        for(pos_t i=0; i < M; ++i){
            game.spFinish->add_play();
        }
    }
}

pb_enum Mahjong::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_INVALID;
    switch (bunch.type()) {
        case pb_enum::BUNCH_A:
            if(bunch.pawns_size()==1)
                bt=bunch.type();
            break;
        case pb_enum::BUNCH_AAA:
            if(bunch.pawns_size()==3){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                auto C=bunch.pawns(2);
                if(A/1000==B/1000&&A/1000==C/1000&&
                   A%100==B%100&&A%100==C%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::BUNCH_AAAA:
            if(bunch.pawns_size()==4){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                auto C=bunch.pawns(2);
                auto D=bunch.pawns(3);
                if(A/1000==B/1000&&A/1000==C/1000&&A/1000==D/1000&&
                   A%100==B%100&&A%100==C%100&&A%100==D%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::OP_PASS:
        case pb_enum::BUNCH_WIN:
            bt=bunch.type();
        default:
            break;
    }
    bunch.set_type(bt);
    return bt;
}

bool Mahjong::verifyDiscard(Game& game,bunch_t& bunch){
    if(bunch.pawns_size()!=1)
        return false;
    auto& gdata=game.players[bunch.pos()]->playData;
    //huazhu
    auto B=gdata.selected_card();
    auto A=bunch.pawns(0);
    if(A/1000!=B/1000){
        for(auto card:gdata.hands()){
            if(card/1000==B/1000)
                return false;
        }
    }
    return true;
}

bool Mahjong::validId(uint id){
    auto color=id/1000;
    if(color<1||color>4)return false;
    auto value=id%100;
    if(value<1||value>9)return false;
    return true;
}

void Mahjong::test(){
    Mahjong ddz;
    Game game;
    ddz.deal(game);
    proto3::bunch_t A,B;
    A.set_pos(0);
    B.set_pos(1);
    std::vector<uint> va{5,6,7,8,9};
    std::vector<uint> vb{4,5,6,7,8};
    ddz.make_bunch(A,va);
    ddz.make_bunch(B,vb);
    
    ddz.verifyBunch(game,A);
    ddz.verifyBunch(game,B);
}


