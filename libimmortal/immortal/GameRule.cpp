//
//  GameRule.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "ImmortalFwd.h"
#include <random>
#include <algorithm>
using namespace proto3;

inline void parseCardsByString(std::vector<int>& o,const std::string& line);

void GameRule::deal(Game& game){
    Logger<<"game "<<(int)game.id<<" begin, type("<<Type()<<")"<<endl;

    auto MP=MaxPlayer(game);
    //clear
    changePos(game,game.banker);
    game.banker=game.token;
    game.pile.clear();
    game.bottom.clear();
    game.pileMap.clear();
    game.historical.clear();
    game.pendingMeld.clear();
    game.pendingDiscard.reset();
    for(auto& player:game.players)if(player)player->reset();
    
    if(!game.spFinish){
        //prepare final end message
        game.spFinish=std::make_shared<MsgNCFinish>();
        for(pos_t i=0; i < MP; ++i)game.spFinish->add_play();
    }
    
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
        auto C=std::min((int)o.size(),maxCards(game));
        for(size_t i=0,ii=C;i!=ii;++i){
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
    auto MH=maxHands(game);
    auto BK=MH+bottom(game);
    auto sorter=std::bind(&GameRule::comparision,this,std::placeholders::_1,std::placeholders::_2);
    for(auto x=game.pile.begin()+MH,xx=game.pile.begin()+BK;x!=xx;++x)game.bottom.push_back(*x);
    for(int i=0;i<MP;++i){
        size_t pos=(game.banker+i)%MP;
        size_t ibeg=(i==0?0:BK+MH*(i-1));
        size_t iend=+BK+MH*i;
        if(pos==game.banker)
            game.firstCard=*(game.pile.begin()+iend-1);
        std::sort( game.pile.begin()+ibeg,    game.pile.begin()+iend, sorter);
        for(auto x=game.pile.begin()+ibeg, xx=game.pile.begin()+iend; x!=xx;++x)game.players[pos]->playData.mutable_hands()->Add(*x);
    }
    game.pile.erase(game.pile.begin(),game.pile.begin()+BK+MH*(MP-1));
    for(auto x=game.pile.begin(),xx=game.pile.end();x!=xx;++x)game.pileMap[*x]=0;
    for(auto i=0;i<MP;++i)logHands(game,i,"deal");
    
    std::string str;
    google::protobuf::RepeatedField<int> pbpile(game.pile.begin(),game.pile.end());
    cards2str(str,pbpile);
    Logger<<"pile: "<<str.c_str()<<endl;

    game.lastCard=game.pile.front();

    //init game replay
    auto spReplay=std::make_shared<replay>();
    spReplay->set_round(game.round);
    spReplay->set_banker(game.banker);
    spReplay->set_gameid(game.id);
    spReplay->set_timestamp((unsigned)time(nullptr));
    for(auto c:game.pile)spReplay->add_piles(c);
    for(auto b:game.bottom)spReplay->add_bottom(b);
    for(auto p:game.players)spReplay->add_hands()->mutable_pawns()->CopyFrom(p->playData.hands());
    game.spReplay=spReplay;
    
    //broadcast
    MsgNCDeal msg;
    msg.set_mid(pb_msg::MSG_NC_DEAL);
    msg.set_banker(game.banker);
    msg.set_ante(10);
    msg.set_multiple(1);
    for(int i=0;i<MP;++i)
        msg.mutable_count()->Add((int)game.players[i]->playData.hands().size());
    for(auto b:game.bottom)msg.add_bottom(b);
    msg.set_piles((int)game.pile.size());

    for(auto p:game.players){
        msg.set_pos(p->playData.seat());
        auto hands=msg.mutable_hands();
        auto n=(int)p->playData.hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,p->playData.hands(j));
        
        p->send(msg);
        hands->Clear();
    }
}

bool GameRule::settle(Game& game){
    Logger<<"game "<<(int)game.id<<" round "<<game.round<<" end"<<endf;

    //broadcast
    game.spSettle=std::make_shared<MsgNCSettle>();
    game.spSettle->clear_play();
    
    //copy game data
    for(int i=0;i<MaxPlayer(game);++i)
        game.spSettle->add_play()->CopyFrom(game.players[i]->playData);
    //copy pile
    for(auto c:game.pile)game.spSettle->mutable_pile()->Add(c);
    
    //persistence replay
    persistReplay(game);
    
    ++game.round;
    
    //permanent data
    bool cost_failed=false;
    if(game.round==1){
        //cost gold
        auto owner=game.players[0]->playData.mutable_player();
        auto uid=owner->uid().c_str();
        char key[64],field[32];
        sprintf(key,"player:%s",uid);
        
        auto spdb=Immortal::sImmortal->spdb;
        auto is_free=(int)Immortal::sImmortal->config.value("free");
        if(!is_free){
            int gold=0;
            int cost=1;
            spdb->lock(uid);
            {
                char gold_key[128];
                std::string str;
                sprintf(gold_key,"player:%s",uid);
                spdb->hget(gold_key,"gold",str);
                gold=atoi(str.c_str());
                
                if(gold>=cost){
                    owner->set_gold(gold-cost);
                    spdb->hincrby(gold_key,"gold",gold-cost);
                }else{
                    cost_failed=true;
                    Logger<<"game "<<(int)game.id<<" no enough gold "<<gold<<endf;
                }
            }
            spdb->unlock(uid);
        }
        
        //stats
        sprintf(key,"game_count:%d",game.rule->Type());
        sprintf(field,"%d",(int)game.category);
        spdb->hincrby(key,field);
    }
    
    //just send
    auto& msg=*game.spSettle;
    if(cost_failed){
        //no data for cheater
        msg.Clear();
        msg.set_result(pb_enum::ERR_NOENOUGH);
    }
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    for(auto p:game.players){
        p->send(msg);
        p->lastMsg=game.spSettle;
    }
    
    //end round
    if(game.round>=game.Round){
        if(!game.spFinish)return false;
        
        auto& fin=*game.spFinish;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players){
            p->send(fin);
            p->lastMsg=game.spFinish;
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
        omsg.set_pos(player.playData.seat());
        omsg.set_result(pb_enum::SUCCEESS);
        for(auto& p:game->players)p->send(omsg);
    }
}

void GameRule::OnEngage(Player& player,uint key){
    if(auto game=player.game){
        if(!player.engaged){
            player.engaged=true;
            player.playData.set_selected_card(key);
        }

        int engaged=0;
        for(auto& p:game->players)if(p->engaged)engaged++;

        if(game->state==Game::State::ST_ENGAGE && engaged>=MaxPlayer(*game)){
            MsgNCEngage omsg;
            omsg.set_mid(pb_msg::MSG_NC_ENGAGE);
            omsg.set_result(pb_enum::SUCCEESS);
            omsg.mutable_keys()->Resize(MaxPlayer(*game),1001);
            for(int i=0;i<MaxPlayer(*game);++i)omsg.add_bunch();
            engage(*game,omsg);

            for(auto& p:game->players)p->send(omsg);
            
            char buf[64];
            sprintf(buf,"%s","all engaged:");
            for(auto& key:omsg.keys())sprintf(buf,"%s,%d",buf,key);
            Logger<<buf<<"\n";
        }
    }
}

bool GameRule::Ready(Game& game){
    int n=0;
    for(auto p:game.players)if(p&&p->ready)++n;
    auto ready=(n>=MaxPlayer(game));
    //reset after all ready
    if(ready)for(auto p:game.players)if(p)p->ready=false;
    return ready;
}

void GameRule::changePos(Game& game,pos_t pos){
    auto old=game.token;
    pos=pos%game.rule->MaxPlayer(game);
    if(game.token!=pos){
        game.token=pos;
        Logger<<" "<<old<<"=>"<<game.token<<endl;
    }
}

void GameRule::changeState(Game& game,Game::State state){
    if(game.state!=state){
        std::string str0,str1;
        Logger<<" "<<state2str(str0,game.state)<<"=>"<<state2str(str1,state)<<endl;
        game.state=state;
    }
}

void GameRule::logHands(Game& game,uint32 pos,std::string msg){
    std::string str;
    auto& hands=game.players[pos]->playData.hands();
    cards2str(str,hands);
    Logger<<msg.c_str()<<" hand of "<<pos<<":"<<hands.size()<<","<<str.c_str()<<endl;
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
    if(bunch.child_size()>0){
        str+="{";
        for(auto& ch:bunch.child()){
            sprintf(buf,"ops=%d,card=",(int)ch.type());
            std::string str0;
            cards2str(str0,ch.pawns());
            str+=buf+str0;
        }
        str+="}";
    }
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

void GameRule::persistReplay(Game& game){
    if(!game.spReplay)return;
    
    std::string replaybuf;
    if(game.spReplay->SerializeToString(&replaybuf)){
        //replay hash data - replay:<game id>{round:data}
        char key[32],field[32];
        sprintf(key,"replay:%d",game.id);
        sprintf(field,"%d",game.round);
        Immortal::sImmortal->spdb->hset(key,field,replaybuf.c_str());
        Logger<<"----replay data len="<<(int)replaybuf.size()<<endl;
    }
}

void GameRule::release(Game& game){
    //player replay list - replay:player:<uid>{replays list}
    replays all;
    all.set_gameid(game.id);
    all.set_gamecategory(game.category);
    all.set_rounds(game.round+1);
    all.set_max_round(game.Round);
    
    std::string replaybuf;
    if(all.SerializeToString(&replaybuf)){
        char key[32];
        std::vector<std::string> ll;
        ll.push_back(replaybuf);
        
        for(auto p:game.players){
            sprintf(key,"replay:player:%s",p->playData.player().uid().c_str());
            Immortal::sImmortal->spdb->lpush(key,ll);
        }
    }
    Immortal::sImmortal->removeGame(game.id);
}

void parseCardsByString(std::vector<int>& o,const std::string& str){
    char c=',';
    auto line(str);
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
