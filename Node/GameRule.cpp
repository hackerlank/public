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

inline void parseCardsByString(std::vector<int>& o,std::string& line);

void GameRule::deal(Game& game){
    //clear
    changePos(game,game.banker);
    game.banker=game.token;
    game.pile.clear();
    game.historical.clear();
    game.pendingMeld.clear();
    game.pendingDiscard.reset();
    for(auto& player:game.players)if(player)player->reset();
    
    //init cards
    initCard(game);
    
    //shuffle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(game.pile.begin(),game.pile.end(),gen);
    std::shuffle(game.pile.begin(),game.pile.end(),gen);

    if(!game.definedCards.empty()){
        std::vector<int> o;
        parseCardsByString(o,game.definedCards);
        for(size_t i=0,ii=o.size();i!=ii;++i){
            auto I=o[i];
            for(size_t j=0,jj=game.pile.size();j!=jj;++j){
                auto J=game.pile[j];
                if(I==J&&i!=j){
                    //swap
                    std::swap(game.pile[i],game.pile[j]);
                    break;
                }
            }
        }
    }
    
    //deal: fixed position,movable banker
    auto MP=MaxPlayer();
    auto MH=maxHands();
    auto BK=MH+bottom();
    bunch_t bottom;
    auto sorter=std::bind(&GameRule::comparision,this,std::placeholders::_1,std::placeholders::_2);
    for(auto x=game.pile.begin()+MH,xx=game.pile.begin()+BK;x!=xx;++x)bottom.add_pawns(*x);
    for(int i=0;i<MP;++i){
        size_t pos=(game.banker+i)%MP;
        size_t ibeg=(i==0?0:BK+MH*(i-1));
        size_t iend=+BK+MH*i;
        std::sort( game.pile.begin()+ibeg,    game.pile.begin()+iend, sorter);
        for(auto x=game.pile.begin()+ibeg, xx=game.pile.begin()+iend; x!=xx;++x)game.players[pos]->playData.mutable_hands()->Add(*x);
    }
    game.pile.erase(game.pile.begin(),game.pile.begin()+BK+MH*(MP-1));
    for(auto x=game.pile.begin(),xx=game.pile.end();x!=xx;++x)game.pileMap[*x]=0;
    for(auto i=0;i<MP;++i)logHands(game,i,"deal");

    //broadcast
    MsgNCStart msg;
    msg.set_mid(pb_msg::MSG_NC_START);
    msg.set_banker(game.banker);
    msg.set_ante(10);
    msg.set_multiple(1);
    for(int i=0;i<MP;++i)
        msg.mutable_count()->Add((int)game.players[i]->playData.hands().size());
    msg.mutable_bottom()->CopyFrom(bottom.pawns());
    
    for(auto p:game.players){
        msg.set_pos(p->pos);
        auto hands=msg.mutable_hands();
        auto n=(int)game.players[p->pos]->playData.hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.players[p->pos]->playData.hands(j));
        
        p->send(msg);
        hands->Clear();
    }
    
    //first discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(game.token);
}

bool GameRule::isGameOver(Game& game){
    for(auto player:game.players){
        if(player->playData.hands().size()<=0)
            return true;
    }
    return false;
}

bool GameRule::settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.players[i]->playData;
        if(gd.hands().size()<=0)
            pos=i;
    }
    
    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto play=msg.mutable_play(i);
        play->set_win(i==pos?1:0);
        play->mutable_hands()->CopyFrom(game.players[i]->playData.hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players){
        p->send(msg);
        p->lastMsg=std::make_shared<MsgNCSettle>(msg);
    }
    
    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players){
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCFinish>(fin);
        }
        return true;
    }
    return false;
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

void GameRule::OnEngage(Player& player,uint key){
    if(auto game=player.game){
        if(player.engaged)return;
        
        player.engaged=true;
        player.playData.set_selected_card(key);
        
        MsgNCEngage omsg;
        omsg.set_mid(pb_msg::MSG_NC_ENGAGE);
        omsg.set_pos(player.pos);
        omsg.set_key(key);
        omsg.set_result(pb_enum::SUCCEESS);
        for(auto& p:game->players)p->send(omsg);
    }
}

bool GameRule::Engaged(Game& game){
    int n=0;
    for(auto p:game.players)if(p&&p->engaged)++n;
    return n>=MaxPlayer();
}

bool GameRule::Ready(Game& game){
    int n=0;
    for(auto p:game.players)if(p&&p->ready)++n;
    auto ready=(n>=MaxPlayer());
    //reset after all ready
    if(ready)for(auto p:game.players)if(p)p->ready=false;
    return ready;
}

void GameRule::changePos(Game& game,pos_t pos){
    auto old=game.token;
    game.token=pos%game.rule->MaxPlayer();
    KEYE_LOG("token: %d=>%d\n",old,game.token);
}

void GameRule::changeState(Game& game,Game::State state){
    if(game.state!=state){
        std::string str0,str1;
        KEYE_LOG("game state: %s=>%s\n",state2str(str0,game.state),state2str(str1,state));
        game.state=state;
    }
}

void GameRule::logHands(Game& game,uint32 pos,std::string msg){
    std::string str;
    auto& hands=game.players[pos]->playData.hands();
    cards2str(str,hands);
    KEYE_LOG("%s hand of %d:%d %s\n",msg.c_str(),pos,hands.size(),str.c_str());
}

const char* GameRule::state2str(std::string& str,Game::State st){
    switch (st) {
        case Game::State::ST_WAIT:
            str="ST_WAIT";
            break;
        case Game::State::ST_ENGAGE:
            str="ST_ENGAGE";
            break;
        case Game::State::ST_DISCARD:
            str="ST_DISCARD";
            break;
        case Game::State::ST_MELD:
            str="ST_MELD";
            break;
        case Game::State::ST_SETTLE:
            str="ST_SETTLE";
            break;
        case Game::State::ST_END:
            str="ST_END";
            break;
        default:
            str="UNKNOWN";
            break;
    }
    return str.c_str();
}

const char* GameRule::bunch2str(std::string& str,const proto3::bunch_t& bunch){
    char buf[32];
    sprintf(buf,"ops=%d,card=",(int)bunch.type());
    cards2str(str,bunch.pawns());
    str=buf+str;
    return str.c_str();
}

const char* GameRule::cards2str(std::string& str,const google::protobuf::RepeatedField<int>& ids){
    str.clear();
    char buf[32];
    for(auto id:ids){
        sprintf(buf,"%d,",id);
        str+=buf;
    }
    return str.c_str();
}

void GameRule::make_bunch(proto3::bunch_t& bunch,const std::vector<uint>& vals){
    bunch.mutable_pawns()->Clear();
    for(auto id:vals){
        bunch.mutable_pawns()->Add(id);
    }
}

void parseCardsByString(std::vector<int>& o,std::string& line){
    char c=',';
    std::string comma;comma.push_back(c);
    if(!line.empty()&&line.back()!=c)line+=comma;
    while(true){
        auto i=line.find(comma);
        if(i==std::string::npos)break;
        auto str=line.substr(0,i);
        auto id=atoi(str.c_str());
        line=line.substr(++i);
        o.push_back(id);
    }
}
