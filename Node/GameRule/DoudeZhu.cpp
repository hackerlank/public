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
    srand(unsigned(time(nullptr)));
    auto r=rand();
    std::random_shuffle(game.pile.begin(),game.pile.end());
    srand(unsigned(time(nullptr)+r));
    std::random_shuffle(game.pile.begin(),game.pile.end());
    if(r>RAND_MAX/2){
        srand(unsigned(time(nullptr)+2016));
        std::random_shuffle(game.pile.begin(),game.pile.end());
    }
    
    //deal: fixed position,movable banker
    size_t I=game.banker,
    J=(game.banker+1)%MaxPlayer(),
    K=(game.banker+2)%MaxPlayer();
    for(auto x=game.pile.begin(),       xx=game.pile.begin()+20;    x!=xx;++x)game.gameData[I].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20,    xx=game.pile.begin()+20+17; x!=xx;++x)game.gameData[J].mutable_hands()->Add(*x);
    for(auto x=game.pile.begin()+20+17, xx=game.pile.end();         x!=xx;++x)game.gameData[K].mutable_hands()->Add(*x);
    for(int i=0;i<MaxPlayer();++i)logHands(game,i);
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
            do{
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
                if(!check)
                    break;
                
                KEYE_LOG("OnDiscard pos=%d,cards(%d:%d)\n",player.pos,cards[0],game->units[cards[0]].value());
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
            }while(false);

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
    //sort cards
    std::vector<uint32> ids(bunch.pawns().begin(),bunch.pawns().end());
    std::sort(ids.begin(),ids.end(),std::bind(&DoudeZhu::comparision,this,game,std::placeholders::_1,std::placeholders::_2));

    auto len=ids.size();
    auto bt=pb_enum::BUNCH_INVALID;
    std::vector<Card*> cards;
    for(auto c:ids)cards.push_back(&game.units[c]);
    //verify by length
    switch (len) {
        case 1:
            bt=pb_enum::BUNCH_A;
            break;
        case 2:
            if(cards[0]->value()==cards[1]->value())
                bt=pb_enum::BUNCH_AA;
            break;
        case 3:
            if(cards[0]->value()==cards[1]->value()&&cards[0]->value()==cards[2]->value())
                bt=pb_enum::BUNCH_AAA;
            break;
        case 4:
            if(cards[0]->value()==cards[1]->value()&&cards[0]->value()==cards[2]->value()
               &&cards[0]->value()==cards[3]->value())
                bt=pb_enum::BUNCH_AAAA;
            else if(cards[0]->value()==cards[1]->value()&&cards[2]->value()==cards[3]->value()
               &&cards[0]->value()+1==cards[2]->value())
                //AABB
                bt=pb_enum::BUNCH_ABC;
            else{
                for(int i=0;i<4;++i){
                    auto v=cards[i];
                    int dup=0;
                    for(auto u:cards)if(v->value()==u->value())++dup;
                    if(dup==1){
                        //the different one,move to end
                        if(i!=3)std::swap(cards[i],cards[3]);
                        if(cards[0]->value()==cards[1]->value()&&cards[0]->value()==cards[2]->value())
                            bt=pb_enum::BUNCH_AAAB;
                    }
                }
            }
            break;
        default:{
            //more than 5: BUNCH_AAAB,BUNCH_ABC,BUNCH_AAAAB
            std::map<uint32,int> valCount;  //[value,count]
            //collect all counts
            int maxSame=0;
            for(auto card:cards){
                auto val=card->value();
                if(valCount.count(val))
                    valCount[val]++;
                else
                    valCount[val]=1;
                if(valCount[val]>maxSame)
                    maxSame=valCount[val];
            }
            switch (maxSame) {
                case 4:{
                    std::vector<int> counts;
                    for(auto imap:valCount){
                        if(imap.second!=4)
                            counts.push_back(imap.second);
                    };
                    auto lcounts=counts.size();
                    if((lcounts==1&&counts.front()==2)||            //AA
                       (lcounts==2&&counts.front()==counts.back())) //AB,AABB,AAABBB
                        bt=pb_enum::BUNCH_AAAAB;
                    break;
                }
                case 3:
                    if(valCount.size()==2)
                        bt=pb_enum::BUNCH_AAAB;
                    else{
                        //AAABBBCD,AAABBBCCCDEF
                        std::vector<int> counts;
                        int AAA=0;
                        for(auto imap:valCount){
                            if(imap.second!=3)
                                counts.push_back(imap.second);
                            else AAA++;
                        };
                        if(len-AAA*3==AAA)
                            bt=pb_enum::BUNCH_AAAB;
                        else if(AAA==counts.size()){
                            bt=pb_enum::BUNCH_AAAB;
                            for(auto m:counts){
                                for(auto n:counts){
                                    if(m!=n){
                                        //count A,B in AB not match
                                        bt=pb_enum::BUNCH_INVALID;
                                        break;
                                    }
                                }
                                if(bt==pb_enum::BUNCH_INVALID)
                                    break;
                            }
                        }
                    }
                    break;
                case 2:
                    bt=pb_enum::BUNCH_ABC;
                    if(len%2!=0){
                        bt=pb_enum::BUNCH_INVALID;
                    }else for(size_t i=0;i<len-2;){
                        if(cards[i]!=cards[i+1])
                            bt=pb_enum::BUNCH_INVALID;
                        else if(i+2<len&&cards[i]+1!=cards[i+2])
                            bt=pb_enum::BUNCH_INVALID;
                        i+=2;
                    }
                    break;
                case 1:
                default:
                    bt=pb_enum::BUNCH_ABC;
                    for(size_t i=0;i<len-1;++i){
                        if(cards[i]+1!=cards[i+1]){
                            bt=pb_enum::BUNCH_INVALID;
                            break;
                        }
                    }
                    break;
            }//switch
            break;
        }//default
    }//switch
    if(bt==pb_enum::BUNCH_INVALID)
        KEYE_LOG("OnDiscard invalid bunch\n");
    
    //rule check
    auto check=false;
    auto& histCard=game.units[hist.pawns(0)];
    if(bt==pb_enum::BUNCH_AAAA){
        //bomb
        if(hist.type()==pb_enum::BUNCH_AAAA){
            if(histCard.value()<cards[0]->value())
                check=true;
        }else
            check=true;
    }else if(bt==hist.type()){
        if(histCard.value()<cards[0]->value())
            check=true;
        else if(histCard.value()==cards[0]->value()){
            check=true;
        }
    }
    if(!check)
        KEYE_LOG("OnDiscard invalid rule\n");
    return check;
}

int DoudeZhu::comparision(Game& game,uint x,uint y){
    auto cx=game.units[x];
    auto cy=game.units[y];
    if(cx.value()==1||cx.value()==2)
        x+=(53-2);
    else if(cx.value()==14)
        x+=8;
    if(cy.value()==1||cy.value()==2)
        y+=(53-2);
    else if(cy.value()==14)
        y+=8;
    
    if(x>y)
        return -1;
    else if(x<y)
        return 1;
    return 0;
}

void DoudeZhu::logHands(Game& game,uint32 pos){
    char buf[32];
    std::string str;
    for(auto c:game.gameData[pos].hands()){
        sprintf(buf,"(%d:%d),",c,game.units[c].value());
        str+=buf;
    }
    KEYE_LOG("hand of %d: %s\n",pos,str.c_str());
}


