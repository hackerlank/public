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

void Mahjong::Tick(Game& game){
    switch (game.state) {
        case Game::State::ST_WAIT:
            if(Ready(game)){
                changeState(game,Game::State::ST_ENGAGE);
                deal(game);
            }
            break;
        case Game::State::ST_ENGAGE:
            if(Engaged(game))
                changeState(game,Game::State::ST_DISCARD);
            break;
        case Game::State::ST_DISCARD:
            //OnDiscard
            break;
        case Game::State::ST_MELD:
            //OnMeld
            break;
        case Game::State::ST_SETTLE:
            if(settle(game))
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

void Mahjong::deal(Game& game){
    GameRule::deal(game);
    for(auto p:game.players)p->engaged=false;
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

void Mahjong::OnMeld(Player& player,const proto3::bunch_t& curr){
    auto pos=player.pos;
    auto spgame=player.game;
    if(!spgame){
        KEYE_LOG("OnMeld no game\n");
        return;
    }
    auto& game=*spgame;
    //state
    if(game.state!=Game::State::ST_MELD){
        KEYE_LOG("OnMeld wrong st=%d,pos=%d\n",game.state,pos);
        return;
    }
    
    //pending queue
    auto& pendingMeld=game.pendingMeld;
    if(pendingMeld.empty()){
        KEYE_LOG("OnMeld with queue empty,pos=%d\n",pos);
        return;
    }

    //pos
    bool found=false;
    int i=0;
    for(;i<pendingMeld.size();++i)
        if(pos==pendingMeld[i].bunch.pos()){
            found=true;
            break;
        }
    if(!found){
        KEYE_LOG("OnMeld wrong player pos=%d\n",pos);
        return;
    }
    
    //arrived and duplicated
    auto& pending=pendingMeld[i];
    if(pending.arrived){
        KEYE_LOG("OnMeld already arrived, pos=%d\n",pos);
        return;
    }
    pending.arrived=true;

    if(pending.bunch.pawns_size()<=0){
        KEYE_LOG("OnMeld empty cards,pos=%d\n",pos);
        return;
    }

    //card or just pass
    int card=-1;
    if(curr.type()!=pb_enum::OP_PASS){
        if(curr.pawns().empty()){
            KEYE_LOG("OnMeld empty cards,pos=%d\n",pos);
            return;
        }
        card=*curr.pawns().rbegin();
        
        auto pcard=*pending.bunch.pawns().rbegin();
        if(card!=pcard){
            KEYE_LOG("OnMeld wrong card=%d,need=%d,pos=%d\n",pcard,card,pos);
            return;
        }
    }

    //queue in
    std::string str;
    //KEYE_LOG("OnMeld queue in,pos=%d,%s\n",pos,bunch2str(str,curr));
    auto ops=pending.bunch.type();
    pending.bunch.CopyFrom(curr);
    //restore pending ops for draw
    if(pending.bunch.type()==pb_enum::OP_PASS)
        pending.bunch.set_type(ops);
    
    int ready=0;
    for(auto& p:pendingMeld)if(p.arrived)++ready;
    if(ready>=pendingMeld.size()){
        //sort
        std::sort(pendingMeld.begin(),pendingMeld.end(),std::bind(&Mahjong::comparePending,this,std::placeholders::_1,std::placeholders::_2));
        
        //priority
        auto& front=pendingMeld.front();
        auto& bunch=front.bunch;
        auto where=bunch.pos();
        auto& who=*game.players[where];

        //ok,verify
        MsgNCMeld msg;
        msg.set_mid(pb_msg::MSG_NC_MELD);
        msg.set_result(pb_enum::SUCCEESS);
        KEYE_LOG("OnMeld pos=%d,%s,token=%d\n",where,bunch2str(str,bunch),game.token);

        auto isDraw=(pendingMeld.size()==1);
        switch(bunch.type()){
            case pb_enum::BUNCH_WIN:{
                std::vector<bunch_t> output;
                if(isGameOver(game,where,card,output)){
                    who.playData.clear_hands();
                }
                break;
            }
            case pb_enum::OP_PASS:
                //handle pass, ensure token
                if(isDraw)
                    bunch.set_pos(game.token);
                else
                    bunch.set_pos(-1);
                break;
            case pb_enum::BUNCH_INVALID:
                //invalid
                KEYE_LOG("OnMeld error, unknown ops, pos=%d\n",where);
                msg.set_result(pb_enum::BUNCH_INVALID);
                break;
            case pb_enum::BUNCH_A:
                //collect after draw
                who.playData.mutable_hands()->Add(card);
                //pending discard
                game.pendingDiscard=std::make_shared<Game::pending_t>();
                game.pendingDiscard->bunch.set_pos(where);
                //remove from pile map
                game.pileMap.erase(card);
                break;
            default:{
                //verify
                auto old_ops=bunch.type();
                auto result=verifyBunch(game,*(bunch_t*)&bunch);
                if(result==pb_enum::BUNCH_INVALID){
                    std::string str;
                    KEYE_LOG("OnMeld verify failed,bunch=%s, old_ops=%d, pos=%d\n",bunch2str(str,bunch),old_ops,where);
                    msg.set_result(pb_enum::BUNCH_INVALID);
                }else{
                    //erase from hands
                    auto& hands=*who.playData.mutable_hands();
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
                    auto h=who.playData.add_bunch();
                    h->CopyFrom(bunch);
                    //pending discard
                    game.pendingDiscard=std::make_shared<Game::pending_t>();
                    game.pendingDiscard->bunch.set_pos(where);
                    changePos(game,where);
                }//meld
            }//default
        }//switch

        //change state before send message
        auto needDraw=false;
        if(bunch.type()==pb_enum::OP_PASS){
            if(isDraw){
                //draw pass to discard
                //KEYE_LOG("OnMeld pass to discard\n");
                changeState(game,Game::State::ST_DISCARD);
                //pending discard
                game.pendingDiscard=std::make_shared<Game::pending_t>();
                game.pendingDiscard->bunch.set_pos(game.token);
            }else{
                //discard pass to draw,don't do it immediately!
                needDraw=true;
            }
        }else if(GameRule::isGameOver(game))
            changeState(game,Game::State::ST_SETTLE);
        else //A,AAA,AAAA
            changeState(game,Game::State::ST_DISCARD);
        msg.mutable_bunch()->CopyFrom(bunch);
        
        //clear after copy
        game.pendingMeld.clear();
        
        //then send
        for(auto p:game.players){
            //need reply all
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCMeld>(msg);
        }
        
        //then draw
        if(needDraw){
            //KEYE_LOG("OnMeld pass to draw\n");
            draw(game);
            changeState(game,Game::State::ST_MELD);
        }
    }//if(ready>=queue.size())
}

void Mahjong::draw(Game& game){
    changePos(game,game.token+1);
    auto player=game.players[game.token];
    auto card=game.pile.back();
    game.pile.pop_back();
    KEYE_LOG("draw pos=%d, card %d\n",game.token,card);

    //game.pendingMeld.clear();
    game.pendingMeld.push_back(Game::pending_t());

    MsgNCDraw msg;
    msg.set_mid(pb_msg::MSG_NC_DRAW);
    msg.set_pos(game.token);
    for(int i=0;i<MaxPlayer();++i){
        auto p=game.players[i];
        if(i==game.token){
            msg.set_card(card);

            //pending meld
            auto& pending=game.pendingMeld.back();
            pending.bunch.set_pos(game.token);
            pending.bunch.set_type(pb_enum::BUNCH_A);   //default ops
            pending.bunch.add_pawns(card);
        }else{
            msg.set_card(card);
//            msg.set_card(-1);
        }
        p->send(msg);
        p->lastMsg=std::make_shared<MsgNCDraw>(msg);
    }
}

bool Mahjong::isGameOver(Game& game,pos_t pos,unit_id_t id,std::vector<proto3::bunch_t>& output){
    auto player=game.players[pos];
    auto& hands=player->playData.hands();
    if(hands.size()<2){
        KEYE_LOG("isGameOver failed: len=%d\n",hands.size());
        return false;
    }
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));
    cards.push_back(id);
    auto sorter=std::bind(&Mahjong::comparision,this,std::placeholders::_1,std::placeholders::_2);
    std::sort(cards.begin(),cards.end(),sorter);
    
    auto len=cards.size()-1;
    for(size_t i=0;i!=len;++i){
        auto A=cards[i+0];
        auto B=cards[i+1];
        if(A/1000==B/1000&&A%100==B%100){
            std::vector<unit_id_t> tmp;
            for(size_t j=0;j!=cards.size();++j)if(j!=i&&j!=i+1)tmp.push_back(cards[j]);
            if(isGameOverWithoutAA(tmp)){
                return true;
            }
        }
    }
    return false;
}

bool Mahjong::isGameOverWithoutAA(std::vector<unit_id_t>& cards){
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

bool Mahjong::hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& src_bunch){
    //for: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
    if(src_bunch.pawns_size()!=1){
        KEYE_LOG("hint wrong cards len=%d,pos=%d\n",src_bunch.pawns_size(),pos);
        return false;
    }
    auto id=src_bunch.pawns(0);
    auto A=id;
    auto& player=*game.players[pos];
    auto& hands=player.playData.hands();
    
    //default color check
    if(A/1000==player.playData.selected_card()){
        KEYE_LOG("hint default color,pos=%d\n",pos);
        return false;
    }
    
    //game over
    std::vector<bunch_t> output;
    if(isGameOver(game,pos,id,output)){
        auto bunch=bunches.Add();
        bunch->set_pos(pos);
        bunch->set_type(pb_enum::BUNCH_WIN);
        bunch->mutable_pawns()->Add(id);
    }

    //select color
    std::vector<unit_id_t> sel;
    for(auto hand:hands){
        auto B=hand;
        if(B/1000==A/1000&&B%100==A%100)
            sel.push_back(hand);
    }
    auto len=sel.size();
    if(len>=2){
        if(len>=3){
            //BUNCH_AAAA
            auto bunch=bunches.Add();
            bunch->set_pos(pos);
            bunch->set_type(pb_enum::BUNCH_AAAA);
            for(int i=0;i<3;++i)bunch->add_pawns(sel[i]);
            bunch->add_pawns(id);
        }
        if(src_bunch.pos()!=pos){
            //BUNCH_AAA, not for self
            auto bunch=bunches.Add();
            bunch->set_pos(pos);
            bunch->set_type(pb_enum::BUNCH_AAA);
            for(int i=0;i<2;++i)bunch->add_pawns(sel[i]);
            bunch->add_pawns(id);
        }
    }else if(game.pileMap.find(id)!=game.pileMap.end()){
        for(auto melt:player.playData.bunch()){
            if(melt.type()==pb_enum::BUNCH_AAA){
                auto C=melt.pawns(0);
                if(C/1000==A/1000&&C%100==A%100){
                    //BUNCH_AAAA
                    auto bunch=bunches.Add();
                    bunch->set_pos(pos);
                    bunch->set_type(pb_enum::BUNCH_AAAA);
                    bunch->mutable_pawns()->CopyFrom(melt.pawns());
                    bunch->add_pawns(id);
                    break;
                }
            }
        }
    }
    
    auto count=bunches.size();
    if(count>0){
        std::string str,ss;
        for(auto& bunch:bunches){
            bunch2str(ss,bunch);
            str+=ss;
        }
        KEYE_LOG("hint %d,pos=%d,%s\n",count,pos,str.c_str());
    }
    return count>0;
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

bool Mahjong::comparePending(Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
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


