//
//  DoudeZhu.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
#include <algorithm>
using namespace proto3;

int DoudeZhu::Type(){
    return pb_enum::RULE_DDZ;
}

int DoudeZhu::MaxPlayer(){
    return 3;
}

bool DoudeZhu::Ready(Game& game){
    return game.ready>=MaxPlayer();
}

void DoudeZhu::Deal(Game& game){
    //clear
    game.token=game.banker;
    game.units.clear();
    game.pile.clear();
    game.gameData.resize(MaxPlayer());
    for(auto& gd:game.gameData)gd.Clear();
    //init cards
    size_t N=54;
    game.units.resize(N);
    game.pile.resize(N);
    unit_id_t id=0;
    for(int i=1;i<=13;++i){ //A-K => 1-13
        for(int j=0;j<4;++j){
            game.pile[id]=id;
            auto& u=game.units[id];
            u.set_color(j); //clubs,diamonds,hearts,spades => 0-3
            u.set_value(i);
            u.set_id(id++);
        }
    }
    for(int j=0;j<=1;++j){  //Joker(color 0,1) => 14
        game.pile[id]=id;
        auto& u=game.units[id];
        u.set_color(j);
        u.set_value(14);
        u.set_id(id++);
    }
    
    //shuffle
    std::random_shuffle(game.pile.begin(),game.pile.end());
    std::random_shuffle(game.pile.begin(),game.pile.end());
    if(rand()>RAND_MAX/2)
        std::random_shuffle(game.pile.begin(),game.pile.end());
    
    //deal: fixed position,movable banker
    size_t I=game.banker,
    J=(game.banker+1)%MaxPlayer(),
    K=(game.banker+2)%MaxPlayer();
    for(auto x=game.pile.begin(),       xx=game.pile.begin()+20;    x!=xx;++x)game.gameData[I].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20,    xx=game.pile.begin()+20+17; x!=xx;++x)game.gameData[J].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20+17, xx=game.pile.end();         x!=xx;++x)game.gameData[K].mutable_hands()->Add(*x);
    //broadcast
    MsgNCStart msg;
    msg.set_mid(pb_msg::MSG_NC_START);
    msg.set_banker(game.banker);
    msg.set_ante(10);
    msg.set_multiple(1);
    auto cards=msg.mutable_cards();
    for(int i=0;i<N;++i){
        auto card=cards->Add();
        card->CopyFrom(game.units[i]);
    }
    for(int i=0;i<MaxPlayer();++i)
        msg.mutable_count()->Add((int)game.gameData[i].hands().size());
    auto bankerHands=game.gameData[I].hands().size();
    for(auto i=bankerHands-3;i<bankerHands;++i)
        msg.mutable_bottom()->Add(game.gameData[I].hands(i));
    
    for(auto p:game.players){
        msg.set_pos(p->pos);
        auto hands=msg.mutable_hands();
        auto n=(int)game.gameData[p->pos].hands().size();
        hands->Resize(n,0);
        for(int j=0;j<n;++j)
            hands->Set(j,game.gameData[p->pos].hands(j));
        
        p->send(msg);
        hands->Clear();
    }
}

void DoudeZhu::OnDiscard(Player& player,MsgCNDiscard& msg){
    MsgNCDiscard omsg;
    omsg.set_mid(pb_msg::MSG_NC_DISCARD);
    omsg.set_result(pb_enum::ERR_FAILED);
    if(auto game=player.game){
        if(game->token==player.pos){
            //verify
            while(true){
                //just pass
                if(msg.bunch().type()==pb_enum::OP_PASS){
                    omsg.set_result(pb_enum::SUCCEESS);
                    break;
                }
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
                
                auto H=game->historical.size();
                check=H<1;
                if(!check){
                    auto hist=&game->historical.back();
                    if(hist->type()==pb_enum::OP_PASS)
                        hist=&game->historical[H-2];
                    if(hist->type()==pb_enum::OP_PASS)
                        check=true;
                    else if(verifyDiscard(*game,*msg.mutable_bunch(),*hist))
                        check=true;
                }
                
                KEYE_LOG("OnDiscard pos=%d,cards=%d\n",player.pos,cards[0]);
                //historic
                game->historical.push_back(msg.bunch());
                //remove hands
                auto& hands=*game->gameData[player.pos].mutable_hands();
                for(auto i=hands.begin();i!=hands.end();){
                    for(auto j:msg.bunch().pawns()){
                        if(j==*i)
                            i=hands.erase(i);
                        else
                            ++i;
                    }
                }
                
                omsg.set_result(pb_enum::SUCCEESS);
                omsg.mutable_bunch()->CopyFrom(msg.bunch());
                break;
            }
            //pass token
            Next(*game);
            
            omsg.mutable_bunch()->set_pos(player.pos);
            for(auto& p:game->players)p->send(omsg);
            return;
        }else
            KEYE_LOG("OnDiscard wrong pos %d(need %d)\n",player.pos,game->token);
    }else
        KEYE_LOG("OnDiscard no game\n");
    player.send(omsg);
}

void DoudeZhu::PostTick(Game& game){
    GameRule::PostTick(game);
    for(auto robot:game.players){
        switch (game.state) {
            case Game::State::ST_DISCARD:
                if(game.token==robot->pos&&robot->isRobot){
                    if(game.delay--<0){
                        KEYE_LOG("tick robot %d\n",robot->pos);

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

void DoudeZhu::Settle(Game& game){
    
}

bool DoudeZhu::IsGameOver(Game& game){
    return false;
}

bool DoudeZhu::Hint(Game& game,pos_t pos,proto3::bunch_t& bunch){
    auto& hands=game.gameData[pos].hands();
    int i=-1;
    auto H=game.historical.size();
    if(H>0){
        proto3::bunch_t* hist=&game.historical.back();
        if(hist->type()==pb_enum::OP_PASS&&H>1)
            hist=&game.historical[H-2];
        if(hist->type()==pb_enum::OP_PASS)
            i=0;
        else{
            auto& histCard=game.units[hist->pawns(0)];
            for(int j=0;j<hands.size();++j){
                auto hand=hands.Get(j);
                if(game.units[hand].value()>histCard.value()){
                    i=j;
                    break;
                }
            }
        }
    }else{
        i=0;
    }
    if(i!=-1){
        bunch.set_pos(pos);
        bunch.set_type(pb_enum::BUNCH_A);
        bunch.mutable_pawns()->Add(hands.Get(i));
        return true;
    }
    
    return false;
}

bool DoudeZhu::verifyDiscard(Game& game,bunch_t& bunch,bunch_t& hist){
    std::vector<uint32> cards(bunch.pawns().begin(),bunch.pawns().end());
    std::sort(cards.begin(),cards.end());

    auto bt=pb_enum::BUNCH_INVALID;
    std::vector<Card*> bunchCard;
    for(auto c:cards)bunchCard.push_back(&game.units[c]);
    switch (cards.size()) {
        case 1:
            bt=pb_enum::BUNCH_A;
            break;
        case 2:
            if(bunchCard[0]->value()==bunchCard[1]->value())
                bt=pb_enum::BUNCH_AA;
            break;
        case 3:
            if(bunchCard[0]->value()==bunchCard[1]->value()&&bunchCard[0]->value()==bunchCard[2]->value())
                bt=pb_enum::BUNCH_AAA;
            break;
        case 4:
            if(bunchCard[0]->value()==bunchCard[1]->value()&&bunchCard[0]->value()==bunchCard[2]->value()&&bunchCard[0]->value()==bunchCard[3]->value())
                bt=pb_enum::BUNCH_AAAA;
            else{
                for(int i=0;i<4;++i){
                    auto v=bunchCard[i];
                    int dup=0;
                    for(auto u:bunchCard)if(v->value()==u->value())++dup;
                    if(dup==1){
                        //the different one,move to end
                        if(i!=3)std::swap(bunchCard[i],bunchCard[3]);
                        if(bunchCard[0]->value()==bunchCard[1]->value()&&bunchCard[0]->value()==bunchCard[2]->value())
                            bt=pb_enum::BUNCH_AAAB;
                    }
                }
            }
            break;
        default:
            //BUNCH_AAAB,BUNCH_ABC,BUNCH_AABBCC,BUNCH_AAAABC
            std::map<uint32,int> valCount;
            int maxval=0;
            for(auto vcard:bunchCard){
                auto val=vcard->value();
                if(valCount.count(val))
                    valCount[val]++;
                else
                    valCount[val]=1;
                if(valCount[val]>maxval)
                    maxval=valCount[val];
            }

            break;
    }
    if(bt==pb_enum::BUNCH_INVALID)
        KEYE_LOG("OnDiscard invalid bunch\n");
    
    //rule check
    auto check=false;
    auto& histCard=game.units[hist.pawns(0)];
    if(bt==pb_enum::BUNCH_AAAA){
        //bomb
        if(hist.type()==pb_enum::BUNCH_AAAA){
            if(histCard.value()<bunchCard[0]->value())
                check=true;
        }else
            check=true;
    }else if(bt==hist.type()){
        if(histCard.value()<bunchCard[0]->value())
            check=true;
        else if(histCard.value()==bunchCard[0]->value()){
            check=true;
        }
    }
    if(!check)
        KEYE_LOG("OnDiscard invalid rule\n");
    return check;
}

