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

int Mahjong::MaxCards(){
    return 108;
}

int Mahjong::MaxHands(){
    return 17;
}

int Mahjong::Bottom(){
    return 1;
}

bool Mahjong::Ready(Game& game){
    return game.ready>=MaxPlayer();
}

void Mahjong::initCard(Game& game){
    unit_id_t id=0;
    //ids => 1111...9999aaaa...iiiiAAAA...III
    for(int i=0;i<3;++i){           //Tong,Suo,Wan
        for(int j=1;j<=9;++j){      //1-9
            for(int k=0;k<4;++k){   //xxxx
                game.pile[id]=id;
                auto& u=game.units[id];
                u.set_color(i);     //Tong,Suo,Wan => 0-3
                u.set_value(i);
                u.set_id(id++);
            }
        }
    }
}

void Mahjong::OnDiscard(Player& player,MsgCNDiscard& msg){
    MsgNCDiscard omsg;
    omsg.set_mid(pb_msg::MSG_NC_DISCARD);
    omsg.set_result(pb_enum::ERR_FAILED);
    
    do{
        auto game=player.game;
        if(!game){
            KEYE_LOG("OnDiscard no game\n");
            break;
        }
        if(game->state!=Game::State::ST_DISCARD){
            KEYE_LOG("OnDiscard wrong state pos %d\n",player.pos);
            break;
        }
        if(game->token!=player.pos){
            KEYE_LOG("OnDiscard wrong pos %d(need %d)\n",player.pos,game->token);
            break;
        }
        msg.mutable_bunch()->set_pos(player.pos);
        
        auto bt=verifyBunch(*game,*msg.mutable_bunch());
        if(pb_enum::BUNCH_INVALID==bt){
            KEYE_LOG("OnDiscard invalid bunch\n");
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
            if(c>=game->units.size()){
                check=false;
                KEYE_LOG("OnDiscard invalid cards %d\n",c);
                break;
            }
            //duplicated id check
            int dup=0;
            for(auto d:cards)if(c==d)dup++;
            if(dup>1){
                check=false;
                KEYE_LOG("OnDiscard duplicated cards %d\n",c);
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
                KEYE_LOG("OnDiscard cards not exists %d\n",c);
                break;
            }
        }
        if(!check)
            break;
        
        std::string str;
        cards2str(*game,str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*game->gameData[player.pos].mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    KEYE_LOG("OnDiscard pos=%d, erase card(%d:%d)\n",player.pos,*i,game->units[*i].value());
                    hands.erase(i);
                    break;
                }
            }
        }
        logHands(*game,player.pos);
        omsg.set_result(pb_enum::SUCCEESS);
        omsg.mutable_bunch()->CopyFrom(msg.bunch());
    }while(false);
    
    if(pb_enum::SUCCEESS==omsg.result()){
        //hints
        for(int i=0;i<MaxPlayer();++i){
            if(i==player.pos)continue;
            
        }

        auto game=player.game;
        omsg.mutable_bunch()->set_pos(player.pos);
        for(auto& p:game->players)p->send(omsg);
        
        //historic
        game->historical.push_back(msg.bunch());

        //pass token
        if(game->gameData[player.pos].hands().size()>0)
            Next(*game);
    }else
        player.send(omsg);
}

void Mahjong::PostTick(Game& game){
    GameRule::PostTick(game);
    for(auto robot:game.players){
        switch (game.state) {
            case Game::State::ST_WAIT:
                if(robot->isRobot)OnReady(*robot);
                break;
            case Game::State::ST_DISCARD:
                if(game.token==robot->pos&&robot->isRobot){
                    if(game.delay--<0){
                        //KEYE_LOG("tick robot %d\n",robot->pos);

                        MsgCNDiscard msg;
                        bunch_t bunch;
                        if(Hint(game,robot->pos,bunch))
                            msg.mutable_bunch()->CopyFrom(bunch);
                        else
                            msg.mutable_bunch()->set_type(pb_enum::OP_PASS);
                        OnDiscard(*robot,msg);
                        game.delay=0;
                    }
                }
                break;
            default:
                break;
        }
    }
}

bool Mahjong::Settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.gameData[i];
        if(gd.hands().size()<=0)
            pos=i;
    }

    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    msg.set_winner(pos);
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto hand=msg.add_hands();
        hand->mutable_pawns()->CopyFrom(game.gameData[i].hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players)p->send(msg);
    
    game.ready=0;
    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players)p->send(msg);
        return true;
    }
    return false;
}

bool Mahjong::IsGameOver(Game& game){
    for(auto gd:game.gameData){
        if(gd.hands().size()<=0)
            return true;
    }
    return false;
}

bool Mahjong::Hint(Game& game,pos_t pos,proto3::bunch_t& bunch){
    //C(17,8) = 24310; C(17,2) = 136
    auto& hands=game.gameData[pos].hands();
    //sort cards
    std::vector<uint32> ids(hands.begin(),hands.end());
    std::sort(ids.begin(),ids.end(),std::bind(&Mahjong::comparision,this,game,std::placeholders::_1,std::placeholders::_2));

    std::vector<uint> ids_;
    auto H=game.historical.size();
    if(H<=0)
        ids_.push_back(ids[0]);
    else{
        proto3::bunch_t* hist=&game.historical.back();
        if(hist->type()==pb_enum::OP_PASS&&H>1)
            hist=&game.historical[H-2];
        auto type=(pb_enum)hist->type();
        if(type==pb_enum::OP_PASS)
            ids_.push_back(ids[0]);
        else{
            std::vector<Card*> cards;
            for(auto c:ids)cards.push_back(&game.units[c]);     //cards vector
            std::vector<Card*> sortByVal[28];                   //redundant vector
            for(auto card:cards)sortByVal[card->value()].push_back(card);
            std::vector<std::vector<Card*>*> sortByWidth[5];    //null,A,AA,AAA,AAAA
            for(auto& sorted:sortByVal)sortByWidth[sorted.size()].push_back(&sorted);

            auto& histCard=game.units[hist->pawns(0)];
            if(type==pb_enum::BUNCH_ABC){
                //make a queue without duplicated
                cards.clear();
                for(auto& v:sortByVal)if(!v.empty()&&v[0]->value()>histCard.value())cards.push_back(v[0]);
                if(!cards.empty()){
                    int len=(int)hist->pawns_size();
                    int y=(int)cards.size()-len;
                    for(int i=0;i<y&&ids_.empty();++i){
                        bunch_t bunch;
                        for(int j=i,jj=i+len;j!=jj;++j)bunch.add_pawns(cards[j]->id());
                        auto bt=verifyBunch(game,bunch);
                        if(bt==type&&compareBunch(game,bunch,*hist)){
                            for(auto card:bunch.pawns())ids_.push_back(card);
                            break;
                        }
                    }
                }
            }else{
                switch(hist->type()){
                    case pb_enum::BUNCH_A:
                    case pb_enum::BUNCH_AA:
                    case pb_enum::BUNCH_AAA:
                    case pb_enum::BUNCH_AAAA:{
                        int idx=1;
                        switch(hist->type()){
                            case pb_enum::BUNCH_AAAA:   idx=4;break;
                            case pb_enum::BUNCH_AAA:    idx=3;break;
                            case pb_enum::BUNCH_AA:     idx=2;break;
                            case pb_enum::BUNCH_A:
                            default:                    idx=1;break;
                        }
                        
                        for(int j=idx;j<5&&ids_.empty();++j){
                            auto& vv=sortByWidth[j];
                            for(auto& v:vv){
                                auto card=v->front();
                                if(card->value()>histCard.value()){
                                    for(auto c:*v)if(ids_.size()<idx)ids_.push_back(c->id());
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case pb_enum::BUNCH_AAAAB:
                        if(!sortByWidth[4].empty()&&sortByWidth[1].size()>=2){
                            auto id0=sortByWidth[1][0]->front()->id();
                            auto id1=sortByWidth[1][1]->front()->id();
                            bunch_t bunch;
                            for(auto sorted:sortByWidth[4]){
                                bunch.mutable_pawns()->Clear();
                                bunch.add_pawns(id0);
                                bunch.add_pawns(id1);
                                for(auto card:*sorted)bunch.add_pawns(card->id());
                                auto bt=verifyBunch(game,bunch);
                                if(bt==type&&compareBunch(game,bunch,*hist)){
                                    for(auto card:bunch.pawns())ids_.push_back(card);
                                    break;
                                }
                            }
                        }
                        break;
                    case pb_enum::BUNCH_AAAB:
                        if(!sortByWidth[3].empty()&&!sortByWidth[1].empty()){
                            auto id=sortByWidth[1][0]->front()->id();
                            bunch_t bunch;
                            for(auto sorted:sortByWidth[3]){
                                bunch.mutable_pawns()->Clear();
                                bunch.add_pawns(id);
                                for(auto card:*sorted)bunch.add_pawns(card->id());
                                auto bt=verifyBunch(game,bunch);
                                if(bt==type&&compareBunch(game,bunch,*hist)){
                                    for(auto card:bunch.pawns())ids_.push_back(card);
                                    break;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }//switch
            }//else if(type==pb_enum::BUNCH_ABC)
            if(hist->type()!=pb_enum::BUNCH_AAAA&&!sortByWidth[4].empty()){
                //boom!
                auto& sorted=sortByWidth[4][0];
                for(auto card:*sorted)bunch.add_pawns(card->id());
            }
        }//else if(type==pb_enum::OP_PASS)
    }


    bunch.set_pos(pos);
    if(!ids_.empty()){
        bunch.set_type(pb_enum::BUNCH_A);
        for(auto id:ids_)bunch.mutable_pawns()->Add(id);
        return true;
    }else{
        bunch.set_type(pb_enum::OP_PASS);
        return false;
    }
}

pb_enum Mahjong::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_A;
    do{
        if(bunch.pawns_size()<=0){
            bt=pb_enum::BUNCH_INVALID;
            break;
        }
        auto& gdata=game.gameData[bunch.pos()];
        auto& A=game.units[bunch.pawns(0)];
        if(gdata.selected_card()!=i_invalid){
            //huazhu
            auto& B=game.units[gdata.selected_card()];
            if(A.color()!=B.color()){
                bt=pb_enum::BUNCH_INVALID;
                break;
            }
        }
    }while (false);
    bunch.set_type(bt);
    return bt;
}

bool Mahjong::compareBunch(Game& game,bunch_t& bunch,bunch_t& hist){
    return true;
}

bool Mahjong::comparision(Game& game,uint x,uint y){
    auto cx=game.units[x];
    auto cy=game.units[y];
    return cx.value()<cy.value();
}

void Mahjong::logHands(Game& game,uint32 pos,std::string msg){
    std::string str;
    auto& hands=game.gameData[pos].hands();
    cards2str(game,str,hands);
    KEYE_LOG("%s hand of %d:%d %s\n",msg.c_str(),pos,hands.size(),str.c_str());
}

void Mahjong::cards2str(Game& game,std::string& str,const google::protobuf::RepeatedField<uint32>& ids){
    str.clear();
    char buf[32];
    for(auto id:ids){
        sprintf(buf,"(%d:%d),",id,game.units[id].value());
        str+=buf;
    }
}

void Mahjong::make_bunch(Game& game,proto3::bunch_t& bunch,const std::vector<uint>& vals){
    bunch.mutable_pawns()->Clear();
    for(auto n:vals){
        uint color=n/100;
        uint val=n%100;
        uint id=0;
        for(auto card:game.units)if(card.value()==val&&card.color()==color)id=card.id();
        bunch.mutable_pawns()->Add(id);
    }
}

void Mahjong::test(){
    Mahjong ddz;
    Game game;
    ddz.Deal(game);
    proto3::bunch_t A,B;
    A.set_pos(0);
    B.set_pos(1);
    std::vector<uint> va{5,6,7,8,9};
    std::vector<uint> vb{4,5,6,7,8};
    ddz.make_bunch(game,A,va);
    ddz.make_bunch(game,B,vb);
    
    ddz.verifyBunch(game,A);
    ddz.verifyBunch(game,B);
    ddz.compareBunch(game,A,B);
}


