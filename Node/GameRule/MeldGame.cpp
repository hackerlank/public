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
            //OnEngage
            break;
        case Game::State::ST_DISCARD:
            //OnDiscard
            break;
        case Game::State::ST_MELD:
            //OnMeld
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
        
        //pass card
        player.unpairedCards.push_back(card);

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
    int i=0;
    for(;i<pendingMeld.size();++i)
        if(pos==pendingMeld[i].bunch.pos())
            break;
    if(i>=pendingMeld.size()){
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

    //need card except pass
    int card=-1;
    if(curr.pawns().empty()){
        if(curr.type()!=pb_enum::OP_PASS){
            KEYE_LOG("OnMeld empty cards,pos=%d\n",pos);
            return;
        }
    }else{
        card=*curr.pawns().rbegin();
        auto pcard=*pending.bunch.pawns().rbegin();
        if(card!=pcard){
            KEYE_LOG("OnMeld wrong card=%d,need=%d,pos=%d\n",pcard,card,pos);
            return;
        }
    }
    //DOTO: use pb_enum::INVALID instead of pb_enum::OP_PASS
    if(curr.type()==pb_enum::OP_PASS && game.pileMap.find(card)==game.pileMap.end())
        player.unpairedCards.push_back(card);

    //queue in
    std::string str;
    //KEYE_LOG("OnMeld queue in,pos=%d,%s\n",pos,bunch2str(str,curr));
    //auto ops=pending.bunch.type();
    pending.bunch.CopyFrom(curr);
    //restore pending ops for draw
    //if(pending.bunch.type()==pb_enum::OP_PASS)pending.bunch.set_type(ops);
    
    int ready=0;
    for(auto& p:pendingMeld)if(p.arrived)++ready;
    if(ready>=pendingMeld.size()){
        //sort
        std::sort(pendingMeld.begin(),pendingMeld.end()
                  ,std::bind(&MeldGame::comparePending,this,game,std::placeholders::_1,std::placeholders::_2));
        
        //priority
        auto& front=pendingMeld.front();
        auto& bunch=front.bunch;
        auto where=bunch.pos();
        auto& who=*game.players[where];

        //ok,verify
        auto old_ops=bunch.type();
        auto result=verifyBunch(game,bunch);
        auto ret=pb_enum::SUCCEESS;
        
        //first deal as pass while invalid
        if(result==pb_enum::BUNCH_INVALID){
            std::string str;
            KEYE_LOG("OnMeld verify failed,bunch=%s, old_ops=%d, pos=%d\n",bunch2str(str,bunch),old_ops,where);
            //result=pb_enum::BUNCH_INVALID;
            result=pb_enum::OP_PASS;
            ret=pb_enum::BUNCH_INVALID;
        }
        
        auto needDraw=false;
        auto isDraw=(pendingMeld.size()==1);
        KEYE_LOG("OnMeld pos=%d,%s,token=%d\n",where,bunch2str(str,bunch),game.token);
        switch(result){
            case pb_enum::BUNCH_WIN:{
                std::vector<bunch_t> output;
                if(isWin(game,who,card,output)){
                    player.playData.clear_hands();
                    settle(player,output,card);
                    changeState(game,Game::State::ST_SETTLE);
                }else{
                    needDraw=!prediscard(who);
                    ret=pb_enum::ERR_FAILED;
                }
                break;
            }
            case pb_enum::OP_PASS:
                //handle pass, ensure token
                if(isDraw){
                    bunch.set_pos(game.token);
                    //draw pass to discard
                    //KEYE_LOG("OnMeld pass to discard\n");
                    auto& currPlayer=*game.players[game.token];
                    needDraw=!prediscard(currPlayer);
                }else{
                    who.discardedCards.push_back(card);
                    
                    bunch.set_pos(-1);
                    //discard pass to draw,don't do it immediately!
                    needDraw=true;
                }
                break;
            case pb_enum::BUNCH_INVALID:
                //checked already
                break;
            default:
                //A,AAA,AAAA, meld or do some specials
                if(meld(game,who,card,bunch))
                    needDraw=!prediscard(who);
                else
                    needDraw=true;
        }

        //change state before send message
        MsgNCMeld msg;
        msg.set_mid(pb_msg::MSG_NC_MELD);
        msg.set_result(ret);
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

void MeldGame::engage(Game& game){
    //after engaged,wait meld to check natural win
    changeState(game,Game::State::ST_MELD);
    
    for(auto p:game.players){
        auto hands=p->playData.mutable_hands();
        auto card=hands->Get(hands->size()-1);
        
        game.pendingMeld.push_back(Game::pending_t());
        auto& pending=game.pendingMeld.back();
        pending.bunch.set_pos(p->pos);
        pending.bunch.add_pawns(card);
    }
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

bool MeldGame::comparision(uint x,uint y){
    auto cx=x/1000;
    auto cy=y/1000;
    if(cx<cy)return true;
    else if(cx==cy)return x%100<y%100;
    else return false;
}

bool MeldGame::comparePending(Game&,Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
}

bool MeldGame::prediscard(Player& player){
    auto& game=*player.game;
    changeState(game,Game::State::ST_DISCARD);
    //pending discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(player.pos);

    return true;
}
