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
            if(Ready(game))
                ChangeState(game,Game::State::ST_START);
            break;
        case Game::State::ST_START:
            deal(game);
            ChangeState(game,Game::State::ST_DISCARD);
            break;
        case Game::State::ST_DISCARD:
            if(game.pendingDiscard&&game.pendingDiscard->arrived)
                ChangeState(game,Game::State::ST_MELD);
            break;
        case Game::State::ST_MELD:
            if(!game.pendingMeld.empty()&&game.pendingMeld.front().arrived){
                auto& pending=game.pendingMeld.front();
                if(pending.bunch.type()==pb_enum::OP_PASS)
                    ChangeState(game,Game::State::ST_DRAW);
                else if(isGameOver(game))
                    ChangeState(game,Game::State::ST_SETTLE);
                else
                    ChangeState(game,Game::State::ST_DISCARD);
                game.pendingMeld.clear();
            }
            break;
        case Game::State::ST_DRAW:
            ChangeState(game,Game::State::ST_MELD);
            draw(game);
            break;
        case Game::State::ST_SETTLE:
            if(settle(game))
                ChangeState(game,Game::State::ST_END);
            else
                ChangeState(game,Game::State::ST_WAIT);
            break;
        case Game::State::ST_END:
            break;
        default:
            break;
    }
    if(game.state<Game::State::ST_SETTLE)
        tickRobot(game);
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

void Mahjong::initCard(Game& game){
    unit_id_t id=0;
    //ids => 1111...9999aaaa...iiiiAAAA...III
    for(int i=0;i<3;++i){           //Tong,Suo,Wan
        for(int j=1;j<=9;++j){      //1-9
            for(int k=0;k<4;++k){   //xxxx
                game.pile[id]=id;
                auto& u=game.units[id];
                u.set_color(i);     //Tong,Suo,Wan => 0-3
                u.set_value(j);
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
        if(card>=game->units.size()){
            KEYE_LOG("OnDiscard invalid cards %d\n",card);
            break;
        }

        //shut discard after verify
        if(!game->pendingDiscard){
            KEYE_LOG("OnDiscard not on pending");
            break;
        }else
            player.game->pendingDiscard.reset();

        std::string str;
        cards2str(*game,str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*player.gameData.mutable_hands();
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

        game->pendingMeld.clear();
        omsg.mutable_bunch()->set_pos(player.pos);
        //hints
        for(int i=0;i<MaxPlayer();++i){
            auto p=game->players[i];
            auto hints=omsg.mutable_hints();
            hints->Clear();
            if(i!=player.pos){
                google::protobuf::RepeatedField<proto3::bunch_t> bunches;
                if(hint(bunches,*game,i,*msg.mutable_bunch())){
                    //add pending meld
                    auto& bunch=bunches.Get(0);
                    game->pendingMeld.push_back(Game::pending_t());
                    auto& pending=game->pendingMeld.back();
                    pending.bunch.CopyFrom(bunch);

                    for(auto& b:bunches)
                        hints->Add()->CopyFrom(b);
                }
            }
            p->send(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
        //pass pending if necessary
        if(game->pendingMeld.empty()){
            game->pendingMeld.push_back(Game::pending_t());
            auto& pending=game->pendingMeld.back();
            pending.bunch.set_type(pb_enum::OP_PASS);
            pending.arrived=true;
            
            bunch_t bunch;
            bunch.add_pawns(msg.bunch().pawns().Get(0));
            bunch.set_type(pb_enum::OP_PASS);
            OnMeld(*game,player,bunch);
        }
        return;
    }while(false);
    player.send(omsg);
}

void Mahjong::OnMeld(Game& game,Player& player,const proto3::bunch_t& bunch){
    auto pos=player.pos;
    //state
    if(game.state!=Game::State::ST_MELD){
        KEYE_LOG("OnMeld wrong st=%d,pos=%d",game.state,pos);
        return;
    }
    
    //pending queue
    auto& queue=game.pendingMeld;
    if(queue.empty()){
        KEYE_LOG("OnMeld with queue empty,pos=%d",pos);
        return;
    }

    //card
    if(bunch.pawns().empty()){
        KEYE_LOG("OnMeld empty cards,pos=%d",pos);
        return;
    }
    auto card=*bunch.pawns().rbegin();
    
    //pos
    bool found=false;
    int i=0;
    for(;i<queue.size();++i)
        if(pos==queue[i].bunch.pos()){
            found=true;
            break;
        }
    if(!found){
        KEYE_LOG("OnMeld wrong player pos=%d",pos);
        return;
    }
    
    //arrived and duplicated
    auto& pending=queue[i];
    if(pending.arrived){
        KEYE_LOG("OnMeld already arrived, pos=%d",pos);
        return;
    }
    pending.arrived=true;

    if(pending.bunch.pawns().empty()){
        KEYE_LOG("OnMeld empty cards,pos=%d",pos);
        return;
    }
    auto pcard=*pending.bunch.pawns().rbegin();
    if(card!=pcard){
        KEYE_LOG("OnMeld wrong card=%d,need=%d,pos=%d",pcard,card,pos);
        return;
    }

    //queue in
    std::string str;
    KEYE_LOG("OnMeld queue in, pos=%d, ops=%s",pos,bunch2str(game,str,bunch));
    pending.bunch.CopyFrom(bunch);
    
    //sort
    std::sort(queue.begin(),queue.end(),std::bind(&Mahjong::comparePending,this,game,std::placeholders::_1,std::placeholders::_2));
    
    //priority
    auto& front=queue.front();
    if(front.arrived){
        //ok,verify
        MsgNCMeld msg;
        msg.set_mid(pb_msg::MSG_NC_MELD);
        msg.set_result(pb_enum::SUCCEESS);

        switch(bunch.type()){
            case pb_enum::BUNCH_WIN:{
                std::vector<bunch_t> output;
                if(isGameOver(game,pos,card,output)){
                    player.gameData.clear_hands();
                }
                break;
            }
            case pb_enum::BUNCH_INVALID:
                //invalid
                KEYE_LOG("OnMeld error, unknown ops, pos=%d",pos);
                msg.set_result(pb_enum::BUNCH_INVALID);
                break;
            case pb_enum::OP_PASS:
                //handle pass
                break;
            default:{
                //verify
                auto old_ops=bunch.type();
                auto result=verifyBunch(game,*(bunch_t*)&bunch);
                if(result==pb_enum::BUNCH_INVALID){
                    std::string str;
                    KEYE_LOG("OnMeld verify failed,bunch=%s, old_ops=%d, pos=%d",bunch2str(game,str,bunch),old_ops,pos);
                    msg.set_result(pb_enum::BUNCH_INVALID);
                }else{
                    //erase from hands
                    auto& hands=*player.gameData.mutable_hands();
                    for(auto j:bunch.pawns()){
                        for(auto i=hands.begin();i!=hands.end();++i){
                            if(j==*i){
                                KEYE_LOG("OnMeld pos=%d, erase card(%d:%d)\n",player.pos,*i,game.units[*i].value());
                                hands.erase(i);
                                break;
                            }
                        }
                    }
                    //then meld
                    auto h=player.gameData.add_bunch();
                    h->CopyFrom(bunch);
                 }//meld
            }//default
        }//switch

        msg.mutable_bunch()->CopyFrom(bunch);
        for(auto p:game.players)p->send(msg);
    }//if front
}

void Mahjong::draw(Game& game){
    if(game.state!=Game::ST_DRAW){
        KEYE_LOG("OnMeld wrong st=%d",game.state);
        return;
    }
    next(game);
    auto player=game.players[game.token];
    auto card=game.pile.back();
    game.pile.pop_back();
    
    MsgNCDraw msg;
    msg.set_mid(pb_msg::MSG_NC_DRAW);
    msg.set_pos(game.token);
    auto pass=false;
    for(int i=0;i<MaxPlayer();++i){
        auto p=game.players[i];
        if(i==game.token){
            msg.set_card(card);

            //pending meld
            game.pendingMeld.clear();
            game.pendingMeld.push_back(Game::pending_t());
            auto& pending=game.pendingMeld.back();
            pending.bunch.set_pos(game.token);

            //hint for the drawer
            google::protobuf::RepeatedField<proto3::bunch_t> bunches;
            bunch_t bunch;
            bunch.set_pos(game.token);
            bunch.mutable_pawns()->Add(card);
            if(hint(bunches,game,i,bunch)){
                pending.bunch.CopyFrom(bunches.Get(0));
                for(auto& b:bunches)if(b.type()!=pb_enum::OP_PASS)msg.add_hints()->CopyFrom(b);
            }
            if(msg.hints_size()<=0){
                pending.bunch.set_type(pb_enum::OP_PASS);
                pass=true;
            }
        }else{
            msg.set_card(i_invalid);
            msg.clear_hints();
        }
        p->send(msg);
    }
    if(pass){
        OnMeld(game,*player,game.pendingMeld.back().bunch);
    }
}

bool Mahjong::isNaturalWin(Game& game,pos_t pos){
    return false;
}

void Mahjong::tickRobot(Game& game){
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
                        auto& gdata=robot->gameData;
                        auto& hands=gdata.hands();
                        if(hands.empty()){
                            break;
                        }else{
                            auto& B=game.units[gdata.selected_card()];
                            auto it=hands.rbegin();
                            //find huazhu if exists
                            for(auto iter=hands.rbegin(),iend=hands.rend();iter!=iend;++iter){
                                auto& A=game.units[*iter];
                                if(A.color()==B.color()){
                                    it=iter;
                                    break;
                                }
                            }
                            msg.mutable_bunch()->add_pawns(*it);
                            OnDiscard(*robot,msg);
                        }
                        game.delay=0;
                    }
                }
                break;
            case Game::State::ST_MELD:
                //show card and wait for accept
                if(!game.pendingMeld.empty()){
                    for(auto i=game.pendingMeld.begin(),ii=game.pendingMeld.end(); i!=ii; ++i)
                        if(i->bunch.pos()==robot->pos){
                            if(i->arrived)
                                //already processed
                                break;
                            auto pmsg=robot->lastMsg.get();
                            if(auto msg=dynamic_cast<MsgNCDiscard*>(pmsg)){
                                auto& hints=msg->hints();
                                if(!hints.empty()){
                                    auto& bunch=hints.Get(0);
                                    OnMeld(game,*robot,bunch);
                                    //KEYE_LOG("ProcessRobot st=%s, pos=%d, suite=%s",st2String(st).c_str(),pos,suite2String(suite).c_str());
                                } else {
                                    //KEYE_LOG("ProcessRobot st=%s, pos=%d, no suite found",st2String(st).c_str(),pos);
                                }
                            }
                            break;
                        }
                }
                break;
            default:
                break;
        }
    }
}

bool Mahjong::settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.players[i]->gameData;
        if(gd.hands().size()<=0)
            pos=i;
    }

    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    msg.set_winner(pos);
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto hand=msg.add_hands();
        hand->mutable_pawns()->CopyFrom(game.players[i]->gameData.hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players){
        p->send(msg);
        p->ready=false;
    }
    
    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players)p->send(msg);
        return true;
    }
    return false;
}

bool Mahjong::isGameOver(Game& game){
    for(auto player:game.players){
        if(player->gameData.hands().size()<=0)
            return true;
    }
    return false;
}

bool Mahjong::isGameOver(Game& game,pos_t pos,unit_id_t id,std::vector<proto3::bunch_t>& output){
    auto player=game.players[pos];
    auto& hands=player->gameData.hands();
    if(hands.size()<2){
        KEYE_LOG("isGameOver failed: len=%d",hands.size());
        return false;
    }
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));
    cards.push_back(id);
    auto sorter=std::bind(&Mahjong::comparision,this,game,std::placeholders::_1,std::placeholders::_2);
    std::sort(cards.begin(),cards.end(),sorter);
    
    auto len=cards.size()-1;
    for(size_t i=0;i!=len;++i){
        auto& A=game.units[cards[i+0]];
        auto& B=game.units[cards[i+1]];
        if(A.color()==B.color()&&A.value()==B.value()){
            std::vector<unit_id_t> tmp;
            for(size_t j=0;j!=len;++j)if(j!=i&&j!=i+1)tmp.push_back(cards[j]);
            if(isGameOverWithoutAA(game,tmp)){
                return true;
            }
        }
    }
    return false;
}

bool Mahjong::isGameOverWithoutAA(Game& game,std::vector<unit_id_t>& cards){
    auto len=cards.size();
    if(len%3!=0){
        KEYE_LOG("isGameOverWithoutAA failed: len=%lu",len);
        return false;
    }
    
    for(size_t i=0,ii=len/3;i!=ii;++i){
        auto& A=game.units[cards[i+0]];
        auto& B=game.units[cards[i+1]];
        auto& C=game.units[cards[i+2]];
        if(A.color()!=B.color()||A.color()!=C.color()){
            KEYE_LOG("isGameOverWithoutAA failed: color");
            return false;
        }
        if(!(A.value()+1==B.value()&&A.value()+2==C.value())&&
           !(A.value()==B.value()&&A.value()==C.value())){
            KEYE_LOG("isGameOverWithoutAA failed: invalid pattern");
            return false;
        }
    }
    
    return true;
}

bool Mahjong::hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& src_bunch){
    //for: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
    if(src_bunch.pawns_size()!=1){
        KEYE_LOG("hint wrong cards len=%d,pos=%d",src_bunch.pawns_size(),pos);
        return false;
    }
    auto id=src_bunch.pawns(0);
    auto A=game.units[id];
    auto& player=*game.players[pos];
    auto& hands=player.gameData.hands();
    
    //default color check
    if(A.color()==player.gameData.selected_card()){
        KEYE_LOG("hint default color,pos=%d",pos);
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
        auto& B=game.units[hand];
        if(B.color()==A.color()&&B.value()==A.value())
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
        //BUNCH_AAA
        auto bunch=bunches.Add();
        bunch->set_pos(pos);
        bunch->set_type(pb_enum::BUNCH_AAA);
        for(int i=0;i<2;++i)bunch->add_pawns(sel[i]);
        bunch->add_pawns(id);
    }else if(game.pileMap.find(id)!=game.pileMap.end()){
        for(auto melt:player.gameData.bunch()){
            if(melt.type()==pb_enum::BUNCH_AAA){
                auto& C=game.units[melt.pawns(0)];
                if(C.color()==A.color()&&C.value()==A.value()){
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
    
    return bunches.size()>0;
}

pb_enum Mahjong::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_A;
    do{
        if(bunch.pawns_size()!=1){
            bt=pb_enum::BUNCH_INVALID;
            break;
        }
        auto& gdata=game.players[bunch.pos()]->gameData;
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

bool Mahjong::verifyDiscard(Game& game,bunch_t& bunch){
    if(bunch.pawns_size()!=1)
        return false;
    auto& gdata=game.players[bunch.pos()]->gameData;
    auto& A=game.units[bunch.pawns(0)];
    if(gdata.selected_card()!=i_invalid){
        //huazhu
        auto& B=game.units[gdata.selected_card()];
        if(A.color()!=B.color())
            return false;
    }
    return true;
}

bool Mahjong::comparision(Game& game,uint x,uint y){
    auto cx=game.units[x];
    auto cy=game.units[y];
    return cx.value()<cy.value();
}

bool Mahjong::comparePending(Game& game,Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
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
    ddz.deal(game);
    proto3::bunch_t A,B;
    A.set_pos(0);
    B.set_pos(1);
    std::vector<uint> va{5,6,7,8,9};
    std::vector<uint> vb{4,5,6,7,8};
    ddz.make_bunch(game,A,va);
    ddz.make_bunch(game,B,vb);
    
    ddz.verifyBunch(game,A);
    ddz.verifyBunch(game,B);
}


