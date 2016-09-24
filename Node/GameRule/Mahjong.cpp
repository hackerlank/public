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
        auto game=player.game;
        omsg.mutable_bunch()->set_pos(player.pos);
        //hints
        for(int i=0;i<MaxPlayer();++i){
            auto p=game->players[i];
            auto hints=omsg.mutable_hints();
            hints->Clear();
            if(i!=player.pos){
                google::protobuf::RepeatedField<proto3::bunch_t> bunches;
                if(Hint(bunches,*game,i,*msg.mutable_bunch()))
                    for(auto& b:bunches)hints->Add()->CopyFrom(b);
            }
            p->send(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
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
                        google::protobuf::RepeatedField<proto3::bunch_t> bunches;
                        if(Hint(bunches,game,robot->pos,*msg.mutable_bunch())){
                            
                        }
//                        OnDiscard(*robot,msg);
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

bool Mahjong::Hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& bunch){
    auto& hands=game.gameData[pos].hands();
    return bunches.size()>0;
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


