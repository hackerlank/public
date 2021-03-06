//
//  MeldGame.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "ImmortalFwd.h"
using namespace proto3;

void MeldGame::Tick(Game& game){
    switch (game.state) {
        case Game::State::ST_WAIT:
            if(IsReady(game)){
                changeState(game,Game::State::ST_ENGAGE);
                deal(game);
            }
            break;
        case Game::State::ST_ENGAGE:
            //OnEngage
            break;
        case Game::State::ST_DISCARD:
            //OnDiscard
            break;
        case Game::State::ST_MELD:
            //OnMeld
            break;
        case Game::State::ST_SETTLE:
            changeState(game,Game::State::ST_WAIT);
            //will change to END
            GameRule::settle(game);
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
    
    auto pos=player.playData.seat();
    do{
        auto game=player.game;
        if(!game){
            Debug<<"OnDiscard no game\n";
            break;
        }
        if(game->token!=pos){
            auto card=(unit_id_t)msg.bunch().pawns(0);
            Debug<<"OnDiscard "<<card<<" wrong pos "<<pos<<"(need "<<game->token<<endl;
            break;
        }
        
        if(game->state!=Game::State::ST_DISCARD){
            Debug<<"OnDiscard wrong state "<<game->state<<",pos="<<pos<<endl;
            break;
        }
        msg.mutable_bunch()->set_pos(pos);
        
        if(!PreDiscard(*game,*msg.mutable_bunch())){
            Debug<<"OnDiscard PreDiscard failed\n";
            break;
        }
        
        //cards check
        auto card=(unit_id_t)msg.bunch().pawns(0);
        //boundary check
        if(!validId(card)){
            Debug<<"OnDiscard invalid cards "<<card<<endl;
            break;
        }

        //shut discard after verify
        if(!game->pendingDiscard){
            Debug<<"OnDiscard not on pending\n";
            break;
        }else
            player.game->pendingDiscard.reset();

        std::string str;
        cards2str(str,msg.bunch().pawns());
        Debug<<pos<<" OnDiscard "<<str.c_str()<<endl;
        //remove hands
        auto& hands=*player.playData.mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    //Debug<<"OnDiscard pos=%d,erase card %d\n",pos,*i);
                    hands.erase(i);
                    break;
                }
            }
        }
        
        //pass discard card
        if(msg.bunch().type()!=pb_enum::BUNCH_A){
            player.unpairedCards.push_back(card);
            Debug<<pos<<" past discard "<<card<<endl;
        }

        auto bDraw=game->pileMap.find(card)!=game->pileMap.end();
        //logHands(*game,pos,"OnDiscard");
        omsg.set_result(pb_enum::SUCCEESS);
        omsg.mutable_bunch()->CopyFrom(msg.bunch());

        //game->pendingMeld.clear();
        omsg.mutable_bunch()->set_pos(pos);
        omsg.mutable_bunch()->set_type(bDraw?pb_enum::BUNCH_A:pb_enum::UNKNOWN);
        
        //ready for meld
        changeState(*player.game,Game::State::ST_MELD);
        //pending meld
        for(int i=0;i<MaxPlayer(*game);++i){
            auto p=game->players[i];
            if(bDraw||i!=pos){
                //only pending others
                game->pendingMeld.push_back(Game::pending_t());
                auto& pending=game->pendingMeld.back();
                pending.bunch.set_pos(i);
                pending.bunch.add_pawns(card);
            }
            //but send to all
            p->send(omsg);
            p->lastMsg=std::make_shared<MsgNCDiscard>(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
        return;
    }while(false);
    PostDiscard(*player.game,omsg);
    player.send(omsg);
}

void MeldGame::OnMeld(Player& player,const proto3::bunch_t& curr){
    auto pos=player.playData.seat();
    auto spgame=player.game;
    if(!spgame){
        Debug<<"OnMeld no game\n";
        return;
    }
    auto& game=*spgame;
    //state
    if(game.state!=Game::State::ST_MELD){
        Debug<<"OnMeld wrong st="<<game.state<<",pos="<<pos<<endl;
        return;
    }
    
    //pending queue
    auto& pendingMeld=game.pendingMeld;
    if(pendingMeld.empty()){
        Debug<<"OnMeld with queue empty,pos="<<pos<<endl;
        return;
    }

    //pos
    int i=0;
    for(;i<pendingMeld.size();++i)
        if(pos==pendingMeld[i].bunch.pos())
            break;
    if(i>=pendingMeld.size()){
        Debug<<"OnMeld wrong player pos="<<pos<<endl;
        return;
    }
    
    //arrived and duplicated
    auto& pending=pendingMeld[i];
    if(pending.arrived){
        Debug<<"OnMeld already arrived, pos="<<pos<<endl;
        return;
    }
    pending.arrived=true;

    if(pending.bunch.pawns_size()<=0){
        Debug<<"OnMeld empty cards,pos="<<pos<<endl;
        return;
    }

    int card=*curr.pawns().begin();

    //force card check
    auto pcard=*pending.bunch.pawns().begin();
    if(pcard!=invalid_card && card!=pcard){
        Debug<<"OnMeld wrong card="<<card<<",need="<<pcard<<",pos="<<pos<<endl;
        return;
    }
    
    //TODO: use pb_enum::INVALID instead of pb_enum::OP_PASS
    //anyway, push to past list
    if(curr.type()==pb_enum::OP_PASS && game.pileMap.find(card)==game.pileMap.end())
        player.unpairedCards.push_back(card);

    //queue in
    std::string str;
    //Debug<<pos<<" OnMeld queue in(total "<<(int)pendingMeld.size()<<"),"<<bunch2str(str,curr)<<endl;
    pending.bunch.CopyFrom(curr);
    
    int ready=0;
    for(auto& p:pendingMeld)if(p.arrived)++ready;
    if(ready>=pendingMeld.size()){
        PreMeld(game);
        
        //sort
        std::vector<bunch_t> bunches;
        sortPendingMeld(spgame,bunches);

        auto bDraw=game.pileMap.find(card)!=game.pileMap.end();
        auto tokenPlayer=game.players[game.token];
        auto ret=pb_enum::SUCCEESS;

        //the highest priority
        auto& front=bunches.front();
        auto which=*front.pawns().begin();
        auto where=front.pos();
        auto who=game.players[where];
        auto from=game.token;

        //change state before send message
        MsgNCMeld msg;
        msg.set_mid(pb_msg::MSG_NC_MELD);
        msg.set_result(ret);
        msg.set_from(from);
        msg.mutable_bunch()->CopyFrom(front);
        
        for(auto& what:bunches){
            //ok,verify
            auto old_ops=what.type();
            auto result=verifyBunch(what);
            auto localPos=what.pos();
            auto localPlayer=game.players[localPos];
            
            if(result>pb_enum::BUNCH_WIN)result=pb_enum::BUNCH_WIN;
            //deal invalid as pass
            if(result==pb_enum::BUNCH_INVALID){
                std::string str;
                Debug<<"OnMeld verify failed,bunch="<<bunch2str(str,what)<<", old_ops="<<old_ops<<", pos="<<localPos<<endl;
                //result=pb_enum::BUNCH_INVALID;
                result=pb_enum::OP_PASS;
                ret=pb_enum::BUNCH_INVALID;
            }
            
            if(what.type()!=pb_enum::OP_PASS)
                Debug<<what.pos()<<" OnMeld "<<bunch2str(str,what)<<",token="<<from<<endl;
            switch(result){
                case pb_enum::BUNCH_WIN:{
                    std::vector<bunch_t> output;
                    if(isWin(game,what,output)){
                        //settle player
                        localPlayer->playData.clear_hands();
                        PreSettle(*localPlayer,&output,which);
                        
                        //reset player bunches
                        localPlayer->playData.clear_bunch();
                        localPlayer->AAAs.clear();
                        localPlayer->AAAAs.clear();
                        for(auto& o:output)localPlayer->playData.mutable_bunch()->Add()->CopyFrom(o);
                        
                        //replay
                        auto op=game.spReplay->add_ops();
                        op->CopyFrom(what);
                        for(auto& o:output)op->add_child()->CopyFrom(o);

                        if(localPos==where){
                            changeState(game,Game::State::ST_SETTLE);
                            tokenPlayer.reset();
                        }
                    }else{
                        //tokenPlayer=who;
                        ret=pb_enum::ERR_FAILED;
                    }
                    break;
                }
                case pb_enum::OP_PASS:
                    //handle pass, ensure token
                    what.set_pos(game.token);
                    if(!bDraw)
                        who->discardedCards.push_back(which);
                    
                    //abandon
                    if(validId(which)){
                        tokenPlayer->playData.add_discards(which);
                        //replay
                        auto op=game.spReplay->add_ops();
                        op->CopyFrom(what);
                    }
                    break;
                case pb_enum::BUNCH_INVALID:
                    //checked already
                    //tokenPlayer.reset();
                    break;
                default:
                    break;
            }   //switch

            //A,AAA,AAAA, meld or do some specials
            if(PostMeld(game,result,from,front,what)){
                tokenPlayer=who;
                changePos(game,who->playData.seat());
            }
        }

        //clear after copy
        game.pendingMeld.clear();
        
        //then send
        for(auto p:game.players){
            //need reply all
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCMeld>(msg);
        }
        
        if(tokenPlayer){
            //then draw or discard
            if(canDiscard(*tokenPlayer,invalid_card))
                discard(player,invalid_card);
            else{
                //Debug<<"OnMeld pass to draw\n");
                changeState(game,Game::State::ST_MELD);
                draw(game);
            }
        }
    }//if(ready>=queue.size())
}

void MeldGame::engage(Game& game,MsgNCEngage&){
    //after engaged,wait meld to check natural win
    changeState(game,Game::State::ST_MELD);
    
    game.pendingMeld.push_back(Game::pending_t());
    auto& pending=game.pendingMeld.back();
    pending.bunch.set_pos(game.banker);
    pending.bunch.add_pawns(invalid_card);
}

void MeldGame::draw(Game& game){
    if(!PreDraw(game) || game.pile.empty()){
        //dismiss
        Debug<<"dismiss while pile empty, pos="<<game.token<<endl;
        Dismiss(game);
    }else{
        changePos(game,game.token+1);
        auto player=game.players[game.token];
        
        auto card=game.pile.back();
        game.pile.pop_back();
        Debug<<game.token<<" draw "<<card<<endl;
        
        //game.pendingMeld.clear();
        game.pendingMeld.push_back(Game::pending_t());
        
        MsgNCDraw msg;
        msg.set_mid(pb_msg::MSG_NC_DRAW);
        msg.set_pos(game.token);
        for(int i=0;i<MaxPlayer(game);++i){
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
                //msg.set_card(-1); //should hide for others
            }
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCDraw>(msg);
        }
        PostDraw(game,card);
    }
}

void MeldGame::sortPendingMeld(std::shared_ptr<Game> spgame,std::vector<proto3::bunch_t>& pending){
    std::sort(spgame->pendingMeld.begin(),spgame->pendingMeld.end()
              ,std::bind(&MeldGame::comparePendingMeld,this,spgame,std::placeholders::_1,std::placeholders::_2));
    pending.push_back(spgame->pendingMeld.front().bunch);
}

bool MeldGame::comparision(unsigned x, unsigned y){
    auto cx=x/1000;
    auto cy=y/1000;
    if(cx<cy)return true;
    else if(cx==cy)return x%100<y%100;
    else return false;
}

bool MeldGame::comparePendingMeld(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
}

void MeldGame::discard(Player& player,unit_id_t card){
    auto& game=*player.game;
    changeState(game,Game::State::ST_DISCARD);
    //pending discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(player.playData.seat());
    game.pendingDiscard->bunch.add_pawns(card);
}
