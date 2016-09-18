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
    MsgNCDiscard omsg;
    omsg.set_mid(pb_msg::MSG_NC_DISCARD);
    omsg.set_result(pb_enum::ERR_FAILED);
    if(auto game=player.game){
        if(game->token==player.pos){
            //verify
            while(true){
                auto& cards=msg.bunch().pawns();
                //cards check
                auto check=true;
                for(auto c:cards){
                    //boundary check
                    if(c>=game->units.size()){
                        check=false;
                        break;
                    }
                    //duplicated id check
                    int dup=0;
                    for(auto d:cards)if(c==d)dup++;
                    if(dup>1){
                        check=false;
                        break;
                    }
                    //exists check
                    auto exist=false;
                    for(auto h:game->gameData[player.pos].hands()){
                        if(h==c){
                            exist=true;
                            break;
                        }
                    }
                    if(!exist){
                        check=false;
                        break;
                    }
                }
                if(!check){
                    KEYE_LOG("OnDiscard invalid cards\n");
                    break;
                }
                
                //bunch check
                auto bt=pb_enum::BUNCH_INVALID;
                std::vector<Card*> V;
                for(auto c:cards)V.push_back(&game->units[c]);
                switch (cards.size()) {
                    case 1:
                        bt=pb_enum::BUNCH_A;
                        break;
                    case 2:
                        if(V[0]->value()==V[1]->value())
                            bt=pb_enum::BUNCH_AA;
                        break;
                    case 3:
                        if(V[0]->value()==V[1]->value()&&V[0]->value()==V[2]->value())
                            bt=pb_enum::BUNCH_AAA;
                        break;
                    case 4:
                        if(V[0]->value()==V[1]->value()&&V[0]->value()==V[2]->value()&&V[0]->value()==V[3]->value())
                            bt=pb_enum::BUNCH_AAAA;
                        else{
                            for(int i=0;i<4;++i){
                                auto v=V[i];
                                int dup=0;
                                for(auto u:V)if(v->value()==u->value())++dup;
                                if(dup==1){
                                    //the different one,move to end
                                    if(i!=3)std::swap(V[i],V[3]);
                                    if(V[0]->value()==V[1]->value()&&V[0]->value()==V[2]->value())
                                        bt=pb_enum::BUNCH_AAAB;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
                if(bt==pb_enum::BUNCH_INVALID){
                    KEYE_LOG("OnDiscard invalid bunch\n");
                    break;
                }
                
                //rule check
                auto H=game->historical.size();
                if(H>0){
                    check=false;
                    proto3::bunch_t* hist=&game->historical.back();
                    if(hist->type()==pb_enum::OP_PASS)
                        hist=&game->historical[H-2];
                    auto& HC=game->units[hist->pawns(0)];
                    if(bt==pb_enum::BUNCH_AAAA){
                        if(hist->type()==pb_enum::BUNCH_AAAA){
                            if(HC.value()<V[0]->value())
                                check=true;
                        }else
                            check=true;
                    }else if(bt==hist->type()){
                        if(HC.value()<V[0]->value())
                            check=true;
                        else if(HC.value()==V[0]->value()){
                            check=true;
                        }
                    }
                }
                if(!check){
                    KEYE_LOG("OnDiscard invalid cards\n");
                    break;
                }
                
                //historic
                game->historical.push_back(msg.bunch());
                
                omsg.set_result(pb_enum::SUCCEESS);
                omsg.mutable_bunch()->CopyFrom(msg.bunch());
                omsg.mutable_bunch()->set_pos(player.pos);
                for(auto& p:game->players)p->send(omsg);
                return;
            }
        }else
            KEYE_LOG("OnDiscard wrong pos %d(need %d)\n",player.pos,game->token);
    }else
        KEYE_LOG("OnDiscard no game\n");
    player.send(omsg);
}

void DoudeZhu::Settle(Game& game){
    
}

bool DoudeZhu::IsGameOver(Game& game){
    return false;
}
