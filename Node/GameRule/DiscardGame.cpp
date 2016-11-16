//
//  DiscardGame.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include <algorithm>
#include "NodeFwd.h"

using namespace proto3;

void DiscardGame::Tick(Game& game){
    switch (game.state) {
        case Game::State::ST_WAIT:
            if(Ready(game)){
                changeState(game,Game::State::ST_ENGAGE);
                deal(game);
            }
            break;
        case Game::State::ST_ENGAGE:
            //OnEngage
            break;
        case Game::State::ST_DISCARD:
            if(isGameOver(game))
                changeState(game,Game::State::ST_SETTLE);
            break;
        case Game::State::ST_SETTLE:
            if(GameRule::settle(game))
                changeState(game,Game::State::ST_END);
            else
                changeState(game,Game::State::ST_WAIT);
            break;
        case Game::State::ST_END:
            break;
        default:
            break;
    }
}

void DiscardGame::engage(Game& game,MsgNCEngage&){
    //after engaged,wait discard
    changeState(game,Game::State::ST_DISCARD);

    //first discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(game.token);
}

void DiscardGame::OnDiscard(Player& player,MsgCNDiscard& msg){
    MsgNCDiscard omsg;
    omsg.set_mid(pb_msg::MSG_NC_DISCARD);
    omsg.set_result(pb_enum::ERR_FAILED);
    
    auto pos=player.playData.seat();
    do{
        auto game=player.game;
        if(!game){
            Logger<<"OnDiscard no game\n";
            break;
        }
        if(game->state!=Game::State::ST_DISCARD){
            Logger<<"OnDiscard wrong state pos "<<pos<<endl;
            break;
        }
        if(game->token!=pos){
            Logger<<"OnDiscard wrong pos "<<pos<<"(need "<<game->token<<")\n";
            break;
        }
        
        //this will fix the type of bunch
        auto bt=verifyBunch(*msg.mutable_bunch());
        if(pb_enum::BUNCH_INVALID==bt){
            Logger<<"OnDiscard invalid bunch\n";
            break;
        }
        
        //just pass
        if(msg.bunch().type()==pb_enum::OP_PASS){
            omsg.set_result(pb_enum::SUCCEESS);
            Logger<<"OnDiscard pos="<<pos<<" pass\n";
            break;
        }

        //verify
        auto& pawns=msg.bunch().pawns();
        std::vector<uint32> cards(pawns.begin(),pawns.end());
        std::sort(cards.begin(),cards.end());
        //cards check
        auto check=true;
        for(auto c:cards){
            //boundary check
            if(!validId(c)){
                check=false;
                Logger<<"OnDiscard invalid cards "<<c<<endl;
                break;
            }
            //duplicated id check
            int dup=0;
            for(auto d:cards)if(c==d)dup++;
            if(dup>1){
                check=false;
                Logger<<"OnDiscard duplicated cards "<<c<<endl;
                break;
            }
            //exists check
            auto exist=false;
            for(auto h:game->players[pos]->playData.hands()){
                if(h==c){
                    exist=true;
                    break;
                }
            }
            if(!exist){
                check=false;
                Logger<<"OnDiscard cards not exists "<<c<<endl;
                break;
            }
        }
        if(!check)
            break;
        
        auto H=game->historical.size();
        check=H<1;
        if(!check){
            auto hist=&game->historical.back();
            if(hist->type()==pb_enum::OP_PASS)
                hist=&game->historical[H-2];
            if(hist->type()==pb_enum::OP_PASS)
                check=true;
            else if(compareBunch(*msg.mutable_bunch(),*hist))
                check=true;
        }
        if(!check){
            Logger<<"OnDiscard compare failed\n";
            break;
        }
        
        std::string str;
        cards2str(str,msg.bunch().pawns());
        Logger<<"OnDiscard pos="<<pos<<",cards "<<str.c_str()<<endl;
        //remove hands
        auto& hands=*game->players[pos]->playData.mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    Logger<<"OnDiscard pos="<<pos<<", erase card "<<*i<<endl;
                    hands.erase(i);
                    break;
                }
            }
        }
        logHands(*game,pos);
        omsg.set_result(pb_enum::SUCCEESS);
        omsg.mutable_bunch()->CopyFrom(msg.bunch());
    }while(false);
    
    if(pb_enum::SUCCEESS==omsg.result()){
        auto game=player.game;
        omsg.mutable_bunch()->set_pos(pos);
        for(auto& p:game->players)p->send(omsg);
        
        //historic
        game->historical.push_back(msg.bunch());

        //pass token
        if(game->players[pos]->playData.hands().size()>0)
            changePos(*game,game->token+1);
    }else
        player.send(omsg);
}

bool DiscardGame::isGameOver(Game& game){
    for(auto player:game.players){
        if(player->playData.hands().size()<=0){
            settle(*player);
            return true;
        }
    }
    return false;
}

bool DiscardGame::comparision(uint x,uint y){
    return x%100<y%100;
}
