//
//  MeldGame.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

void MeldGame::Tick(Game& game){
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

void MeldGame::OnDiscard(Player& player,MsgCNDiscard& msg){
    MsgNCDiscard omsg;
    omsg.set_mid(pb_msg::MSG_NC_DISCARD);
    omsg.set_result(pb_enum::ERR_FAILED);
    
    do{
        auto game=player.game;
        if(!game){
            KEYE_LOG("OnDiscard no game\n");
            break;
        }
        if(game->token!=player.pos){
            KEYE_LOG("OnDiscard wrong pos %d(need %d)\n",player.pos,game->token);
            break;
        }
        
        if(game->state!=Game::State::ST_DISCARD){
            KEYE_LOG("OnDiscard wrong state pos %d\n",player.pos);
            break;
        }
        msg.mutable_bunch()->set_pos(player.pos);
        
        if(!verifyDiscard(*game,*msg.mutable_bunch())){
            KEYE_LOG("OnDiscard invalid bunch\n");
            break;
        }
        
        //cards check
        auto card=(unit_id_t)msg.bunch().pawns(0);
        //boundary check
        if(!validId(card)){
            KEYE_LOG("OnDiscard invalid cards %d\n",card);
            break;
        }

        //shut discard after verify
        if(!game->pendingDiscard){
            KEYE_LOG("OnDiscard not on pending\n");
            break;
        }else
            player.game->pendingDiscard.reset();

        std::string str;
        cards2str(str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*player.playData.mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    //KEYE_LOG("OnDiscard pos=%d,erase card %d\n",player.pos,*i);
                    hands.erase(i);
                    break;
                }
            }
        }
        //logHands(*game,player.pos,"OnDiscard");
        omsg.set_result(pb_enum::SUCCEESS);
        omsg.mutable_bunch()->CopyFrom(msg.bunch());

        //game->pendingMeld.clear();
        omsg.mutable_bunch()->set_pos(player.pos);
        
        //ready for meld
        changeState(*player.game,Game::State::ST_MELD);
        //pending meld
        for(int i=0;i<MaxPlayer();++i){
            auto p=game->players[i];
            if(i!=player.pos){
                //only pending others
                game->pendingMeld.push_back(Game::pending_t());
                auto& pending=game->pendingMeld.back();
                pending.bunch.set_pos(i);
                pending.bunch.add_pawns(card);
            }
            p->send(omsg);
            p->lastMsg=std::make_shared<MsgNCDiscard>(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
        return;
    }while(false);
    player.send(omsg);
}

void MeldGame::OnMeld(Player& player,const proto3::bunch_t& curr){
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
        std::sort(pendingMeld.begin(),pendingMeld.end(),std::bind(&MeldGame::comparePending,this,std::placeholders::_1,std::placeholders::_2));
        
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

void MeldGame::draw(Game& game){
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

bool MeldGame::isNaturalWin(Game& game,pos_t pos){
    return false;
}

bool MeldGame::settle(Game& game){
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

bool MeldGame::isGameOver(Game& game,pos_t pos,unit_id_t id,std::vector<proto3::bunch_t>& output){
    auto player=game.players[pos];
    auto& hands=player->playData.hands();
    if(hands.size()<2){
        KEYE_LOG("isGameOver failed: len=%d\n",hands.size());
        return false;
    }
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));
    cards.push_back(id);
    auto sorter=std::bind(&MeldGame::comparision,this,std::placeholders::_1,std::placeholders::_2);
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

bool MeldGame::isGameOverWithoutAA(std::vector<unit_id_t>& cards){
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

pb_enum MeldGame::verifyBunch(Game& game,bunch_t& bunch){
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

bool MeldGame::verifyDiscard(Game& game,bunch_t& bunch){
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

bool MeldGame::comparision(uint x,uint y){
    auto cx=x/1000;
    auto cy=y/1000;
    if(cx<cy)return true;
    else if(cx==cy)return x%100<y%100;
    else return false;
}

bool MeldGame::comparePending(Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
}
