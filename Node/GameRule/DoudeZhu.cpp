//
//  DoudeZhu.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
#include <random>
#include <algorithm>
using namespace proto3;

inline uint32 transformValue(uint32 val){
    if      (val==1) return 14;
    else if (val==2) return 16;
    else if (val==14)return 18;
    else if (val==15)return 19;
    else             return val;
}

inline uint32 inverseTransformValue(uint32 val){
    if      (val==14)return 1;
    else if (val==16)return 2;
    else if (val==18)return 14;
    else if (val==19)return 15;
    else             return val;
}

int DoudeZhu::Type(){
    return pb_enum::GAME_DDZ;
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
    game.historical.clear();
    game.gameData.resize(MaxPlayer());
    for(auto& gd:game.gameData)gd.Clear();
    
    //init cards
    size_t N=54;
    game.units.resize(N);
    game.pile.resize(N);
    unit_id_t id=0;
    //ids => AAAA22223333...
    for(int i=1;i<=13;++i){ //A-K => 1-13
        for(int j=0;j<4;++j){
            game.pile[id]=id;
            auto& u=game.units[id];
            u.set_color(j); //clubs,diamonds,hearts,spades => 0-3
            u.set_value(transformValue(i));
            u.set_id(id++);
        }
    }
    for(int j=0;j<=1;++j){  //Joker(color 0,1) => 14,15
        game.pile[id]=id;
        auto& u=game.units[id];
        u.set_color(j);
        u.set_value(transformValue(14+j));
        u.set_id(id++);
    }
    
    //shuffle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(game.pile.begin(),game.pile.end(),gen);
    std::shuffle(game.pile.begin(),game.pile.end(),gen);

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

void DoudeZhu::OnReady(Player& player){
    if(auto game=player.game){
        if(player.isRobot&&game->ready>=MaxPlayer()-1)return;

        ++game->ready;
        MsgNCReady omsg;
        omsg.set_mid(pb_msg::MSG_NC_READY);
        omsg.set_pos(player.pos);
        omsg.set_result(pb_enum::SUCCEESS);
        for(auto& p:game->players)p->send(omsg);
    }
}

void DoudeZhu::OnDiscard(Player& player,MsgCNDiscard& msg){
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
        //just pass
        if(msg.bunch().type()==pb_enum::OP_PASS){
            omsg.set_result(pb_enum::SUCCEESS);
            KEYE_LOG("OnDiscard pos=%d pass\n",player.pos);
            break;
        }

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
        
        auto H=game->historical.size();
        check=H<1;
        if(!check){
            auto hist=&game->historical.back();
            if(hist->type()==pb_enum::OP_PASS)
                hist=&game->historical[H-2];
            if(hist->type()==pb_enum::OP_PASS)
                check=true;
            else if(compareBunch(*game,*msg.mutable_bunch(),*hist))
                check=true;
        }
        if(!check){
            KEYE_LOG("OnDiscard compare failed\n");
            break;
        }
        
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
        for(auto& p:game->players)p->send(omsg);
        
        //historic
        game->historical.push_back(msg.bunch());

        //pass token
        if(game->gameData[player.pos].hands().size()>0)
            Next(*game);
    }else
        player.send(omsg);
}

void DoudeZhu::PostTick(Game& game){
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

bool DoudeZhu::Settle(Game& game){
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

bool DoudeZhu::IsGameOver(Game& game){
    for(auto gd:game.gameData){
        if(gd.hands().size()<=16)
            return true;
    }
    return false;
}

bool DoudeZhu::Hint(Game& game,pos_t pos,proto3::bunch_t& bunch){
    //C(17,8) = 24310; C(17,2) = 136
    auto& hands=game.gameData[pos].hands();
    //sort cards
    std::vector<uint32> ids(hands.begin(),hands.end());
    std::sort(ids.begin(),ids.end(),std::bind(&DoudeZhu::comparision,this,game,std::placeholders::_1,std::placeholders::_2));

    int i=-1;
    auto H=game.historical.size();
    if(H>0){
        proto3::bunch_t* hist=&game.historical.back();
        if(hist->type()==pb_enum::OP_PASS&&H>1)
            hist=&game.historical[H-2];
        switch(hist->type()){
            case pb_enum::OP_PASS:
                i=0;
                break;
            case pb_enum::BUNCH_A:{
                auto& histCard=game.units[hist->pawns(0)];
                for(int j=0;j<ids.size();++j){
                    auto hand=ids[j];
                    if(game.units[hand].value()>histCard.value()){
                        i=j;
                        break;
                    }
                }
            }
            default:
                break;
        }
    }else{
        i=0;
    }

    bunch.set_pos(pos);
    if(i!=-1){
        bunch.set_type(pb_enum::BUNCH_A);
        bunch.mutable_pawns()->Add(ids[i]);
        return true;
    }else{
        bunch.set_type(pb_enum::OP_PASS);
        return false;
    }
}

pb_enum DoudeZhu::verifyBunch(Game& game,bunch_t& bunch){
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
            else if(cards[0]->value()>=transformValue(14)&&cards[1]->value()>=transformValue(14))
                // 2 Jokers
                bt=pb_enum::BUNCH_AAAA;
            break;
        case 3:
            if(cards[0]->value()==cards[1]->value()&&cards[0]->value()==cards[2]->value())
                bt=pb_enum::BUNCH_AAA;
            break;
        case 4:
            if(cards[0]->value()==cards[1]->value()&&cards[0]->value()==cards[2]->value()
               &&cards[0]->value()==cards[3]->value())
                bt=pb_enum::BUNCH_AAAA;
            /*
            else if(cards[0]->value()==cards[1]->value()&&cards[2]->value()==cards[3]->value()
               &&cards[0]->value()+1==cards[2]->value())
                bt=pb_enum::BUNCH_ABC;  //AABB
            */
            else{
                //collect all counts
                std::map<uint32,int> valCount;  //[value,count]
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
                if(maxSame==3&&valCount.size()==2)
                    bt=pb_enum::BUNCH_AAAB;
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
                    if(lcounts==1&&counts[0]==2){                  //AA
                        bt=pb_enum::BUNCH_AAAAB;
                    }else if(lcounts==2&&counts[0]==counts[1]){//AB,AABB,AAABBB
                        bt=pb_enum::BUNCH_AAAAB;
                    }
                    break;
                }
                case 3:
                    if(valCount.size()==2&&len<6)
                        //only 1 AAA
                        bt=pb_enum::BUNCH_AAAB;
                    else{
                        //more than 1: AAABBBCD,AAABBBCCCDEF
                        std::vector<int> B;
                        std::vector<uint> vAAA;
                        for(auto imap:valCount){
                            if(imap.second!=3)
                                B.push_back(imap.second);
                            else
                                vAAA.push_back(imap.first);
                        };
                        //adjacent check
                        auto AAA=vAAA.size();
                        auto adjacent=true;
                        for(size_t i=0;i<AAA-1;++i){
                            if(vAAA[i]+1!=vAAA[i+1]){
                                adjacent=false;
                                break;
                            }
                        }
                        if(adjacent){
                            if(len-AAA==AAA*3||len==AAA*3)
                                bt=pb_enum::BUNCH_AAAB;
                            else if(AAA==B.size()){
                                bt=pb_enum::BUNCH_AAAB;
                                for(auto m:B){
                                    for(auto n:B){
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
                    }
                    break;
                case 2:
                    bt=pb_enum::BUNCH_ABC;
                    if(len%2!=0){
                        bt=pb_enum::BUNCH_INVALID;
                    }else for(size_t i=0;i<len-2;){
                        if(cards[i]->value()!=cards[i+1]->value())
                            bt=pb_enum::BUNCH_INVALID;
                        else if(i+2<len&&cards[i]->value()+1!=cards[i+2]->value())
                            bt=pb_enum::BUNCH_INVALID;
                        i+=2;
                    }
                    break;
                case 1:
                    bt=pb_enum::BUNCH_ABC;
                    for(size_t i=0;i<len-1;++i){
                        if(cards[i]->value()+1!=cards[i+1]->value()){
                            bt=pb_enum::BUNCH_INVALID;
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }//switch
            break;
        }//default
    }//switch
    bunch.set_type(bt);
    
    std::string str;
    cards2str(game,str,bunch.pawns());
    //KEYE_LOG("verifyBunch pos=%d,type=%d: %s\n",bunch.pos(),bunch.type(),str.c_str());
    return bt;
}

bool DoudeZhu::compareBunch(Game& game,bunch_t& bunch,bunch_t& hist){
    //rule win
    auto win=false;
    auto bt=bunch.type();
    if(bt==pb_enum::BUNCH_INVALID){
        win=false;
    }else if(bt==pb_enum::BUNCH_AAAA){
        //bomb
        auto& histCard=game.units[hist.pawns(0)];
        auto& bunchCard=game.units[bunch.pawns(0)];
        if(hist.type()!=pb_enum::BUNCH_AAAA||histCard.value()<bunchCard.value())
            win=true;
    }else if(bt==hist.type()&&bunch.pawns().size()==hist.pawns().size()){
        //same type and length
        switch (bt) {
            case pb_enum::BUNCH_AAAB:
            case pb_enum::BUNCH_AAAAB:{
                std::vector<Card*> bunchCards,histCards;
                for(auto c:bunch.pawns())bunchCards.push_back(&game.units[c]);
                for(auto c:hist.pawns())histCards.push_back(&game.units[c]);
                //find value of the same cards
                uint32 bunchVal=0,histVal=0;
                if(pb_enum::BUNCH_AAAB==bt){
                    for(size_t i=0,ii=bunchCards.size()-2;i<ii;++i){
                        if(bunchCards[i]->value()==bunchCards[i+1]->value()&&bunchCards[i]->value()==bunchCards[i+2]->value())
                            bunchVal=bunchCards[i]->value();
                    }
                    for(size_t i=0,ii=histCards.size()-2;i<ii;++i){
                        if(histCards[i]->value()==histCards[i+1]->value()&&histCards[i]->value()==histCards[i+2]->value())
                            histVal=histCards[i]->value();
                    }
                }else{
                    for(size_t i=0,ii=bunchCards.size()-3;i<ii;++i){
                        if(bunchCards[i]->value()==bunchCards[i+1]->value()&&bunchCards[i]->value()==bunchCards[i+2]->value()
                           &&bunchCards[i]->value()==bunchCards[i+3]->value())
                            bunchVal=bunchCards[i]->value();
                    }
                    for(size_t i=0,ii=histCards.size()-3;i<ii;++i){
                        if(histCards[i]->value()==histCards[i+1]->value()&&histCards[i]->value()==histCards[i+2]->value()
                           &&histCards[i]->value()==histCards[i+3]->value())
                            histVal=histCards[i]->value();
                    }
                }
                win=bunchVal>histVal;
                break;
            }
            case pb_enum::BUNCH_A:
            case pb_enum::BUNCH_AA:
            case pb_enum::BUNCH_AAA:
            case pb_enum::BUNCH_ABC:
            default:{
                auto& histCard=game.units[hist.pawns(0)];
                auto& bunchCard=game.units[bunch.pawns(0)];
                if(histCard.value()<bunchCard.value())
                    win=true;
                break;
            }
        }
    }
    std::string str,str1;
    cards2str(game,str,bunch.pawns());
    cards2str(game,str1,hist.pawns());
    //KEYE_LOG("compare win=%d [pos=%d,type=%d: %s] [pos=%d,type=%d: %s]\n",win,bunch.pos(),bunch.type(),str.c_str(),hist.pos(),hist.type(),str1.c_str());
    return win;
}

bool DoudeZhu::comparision(Game& game,uint x,uint y){
    auto cx=game.units[x];
    auto cy=game.units[y];
    /*
    if(cx.value()==1||cx.value()==2)
        x+=(53-2);
    else if(cx.value()==14)
        x+=8;
    if(cy.value()==1||cy.value()==2)
        y+=(53-2);
    else if(cy.value()==14)
        y+=8;
    return x<y;
    */
    return cx.value()<cy.value();
}

void DoudeZhu::logHands(Game& game,uint32 pos,std::string msg){
    std::string str;
    auto& hands=game.gameData[pos].hands();
    cards2str(game,str,hands);
    KEYE_LOG("%s hand of %d:%d %s\n",msg.c_str(),pos,hands.size(),str.c_str());
}

void DoudeZhu::cards2str(Game& game,std::string& str,const google::protobuf::RepeatedField<uint32>& ids){
    str.clear();
    char buf[32];
    for(auto id:ids){
        sprintf(buf,"(%d:%d),",id,game.units[id].value());
        str+=buf;
    }
}

void DoudeZhu::make_bunch(Game& game,proto3::bunch_t& bunch,const std::vector<uint>& vals){
    bunch.mutable_pawns()->Clear();
    for(auto n:vals){
        uint color=n/100;
        uint val=transformValue(n%100);
        uint id=0;
        for(auto card:game.units)if(card.value()==val&&card.color()==color)id=card.id();
        bunch.mutable_pawns()->Add(id);
    }
}

void DoudeZhu::test(){
    DoudeZhu ddz;
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


