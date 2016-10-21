//
//  DoudeZhu.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
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

void DoudeZhu::Tick(Game& game){
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
            if(isGameOver(game))
                changeState(game,Game::State::ST_SETTLE);
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

int DoudeZhu::Type(){
    return pb_enum::GAME_DDZ;
}

int DoudeZhu::MaxPlayer(){
    return 3;
}

int DoudeZhu::maxCards(){
    return 54;
}

int DoudeZhu::maxHands(){
    return 17;
}

int DoudeZhu::bottom(){
    return 3;
}

void DoudeZhu::initCard(Game& game){
    //id: [color-index-value]
    for(int i=1;i<=13;++i){     //value: A-K => 1-13
        for(int j=1;j<=4;++j){  //color: clubs,diamonds,hearts,spades => 1-4
            unit_id_t id=j*1000+transformValue(i);
            game.pile.push_back(id);
        }
    }
    for(int j=1;j<=2;++j){      //Joker(color 1,2) => 14,15
        unit_id_t id=j*1000+transformValue(13+j);
        game.pile.push_back(id);
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
        if(game->state!=Game::State::ST_DISCARD){
            KEYE_LOG("OnDiscard wrong state pos %d\n",player.pos);
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

        auto bt=verifyBunch(*msg.mutable_bunch());
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
            if(!validId(c)){
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
            for(auto h:game->players[player.pos]->gameData.hands()){
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
            else if(compareBunch(*msg.mutable_bunch(),*hist))
                check=true;
        }
        if(!check){
            KEYE_LOG("OnDiscard compare failed\n");
            break;
        }
        
        std::string str;
        cards2str(str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*game->players[player.pos]->gameData.mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    KEYE_LOG("OnDiscard pos=%d, erase card %d\n",player.pos,*i);
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
        if(game->players[player.pos]->gameData.hands().size()>0)
            changePos(*game,game->token+1);
    }else
        player.send(omsg);
}

bool DoudeZhu::settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.players[i]->gameData;
        if(gd.hands().size()<=0)
            pos=i;
    }

    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    for(int i=0,ii=MaxPlayer();i!=ii;++i){
        auto play=msg.mutable_data(i);
        play->set_win(i==pos?1:0);
        play->mutable_hands()->CopyFrom(game.players[i]->gameData.hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players)p->send(msg);

    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players)p->send(msg);
        return true;
    }
    return false;
}

bool DoudeZhu::isGameOver(Game& game){
    for(auto player:game.players){
        if(player->gameData.hands().size()<=0)
            return true;
    }
    return false;
}

bool DoudeZhu::hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& src_bunch){
    //C(17,8) = 24310; C(17,2) = 136
    auto& hands=game.players[pos]->gameData.hands();
    auto& bunch=*bunches.Add();
    bunch.CopyFrom(src_bunch);
    
    //sort cards
    std::vector<uint32> ids(hands.begin(),hands.end());
    std::sort(ids.begin(),ids.end(),std::bind(&DoudeZhu::comparision,this,std::placeholders::_1,std::placeholders::_2));

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
            std::vector<unit_id_t> cards;
            for(auto c:ids)cards.push_back(c);     //cards vector
            std::vector<unit_id_t> sortByVal[28];                   //redundant vector
            for(auto card:cards)sortByVal[card%100].push_back(card);
            std::vector<std::vector<unit_id_t>*> sortByWidth[5];    //null,A,AA,AAA,AAAA
            for(auto& sorted:sortByVal)sortByWidth[sorted.size()].push_back(&sorted);

            auto histCard=hist->pawns(0);
            if(type==pb_enum::BUNCH_ABC){
                //make a queue without duplicated
                cards.clear();
                for(auto& v:sortByVal)if(!v.empty()&&v[0]%100>histCard%100)cards.push_back(v[0]);
                if(!cards.empty()){
                    int len=(int)hist->pawns_size();
                    int y=(int)cards.size()-len;
                    for(int i=0;i<y&&ids_.empty();++i){
                        bunch_t bunch_;
                        for(int j=i,jj=i+len;j!=jj;++j)bunch_.add_pawns(cards[j]);
                        auto bt=verifyBunch(bunch_);
                        if(bt==type&&compareBunch(bunch_,*hist)){
                            for(auto card:bunch_.pawns())ids_.push_back(card);
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
                                if(card%100>histCard%100){
                                    for(auto c:*v)if(ids_.size()<idx)ids_.push_back(c);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case pb_enum::BUNCH_AAAAB:
                        if(!sortByWidth[4].empty()&&sortByWidth[1].size()>=2){
                            auto id0=sortByWidth[1][0]->front();
                            auto id1=sortByWidth[1][1]->front();
                            bunch_t bunch_;
                            for(auto sorted:sortByWidth[4]){
                                bunch_.mutable_pawns()->Clear();
                                bunch_.add_pawns(id0);
                                bunch_.add_pawns(id1);
                                for(auto card:*sorted)bunch_.add_pawns(card);
                                auto bt=verifyBunch(bunch_);
                                if(bt==type&&compareBunch(bunch_,*hist)){
                                    for(auto card:bunch_.pawns())ids_.push_back(card);
                                    break;
                                }
                            }
                        }
                        break;
                    case pb_enum::BUNCH_AAAB:
                        if(!sortByWidth[3].empty()&&!sortByWidth[1].empty()){
                            auto id=sortByWidth[1][0]->front();
                            bunch_t bunch_;
                            for(auto sorted:sortByWidth[3]){
                                bunch_.mutable_pawns()->Clear();
                                bunch_.add_pawns(id);
                                for(auto card:*sorted)bunch_.add_pawns(card);
                                auto bt=verifyBunch(bunch_);
                                if(bt==type&&compareBunch(bunch_,*hist)){
                                    for(auto card:bunch_.pawns())ids_.push_back(card);
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
                for(auto card:*sorted)bunch.add_pawns(card);
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

pb_enum DoudeZhu::verifyBunch(bunch_t& bunch){
    //sort cards
    std::vector<uint32> ids(bunch.pawns().begin(),bunch.pawns().end());
    std::sort(ids.begin(),ids.end(),std::bind(&DoudeZhu::comparision,this,std::placeholders::_1,std::placeholders::_2));

    auto len=ids.size();
    auto bt=pb_enum::BUNCH_INVALID;
    std::vector<unit_id_t> cards;
    for(auto c:ids)cards.push_back(c);
    //verify by length
    switch (len) {
        case 1:
            bt=pb_enum::BUNCH_A;
            break;
        case 2:
            if(cards[0]%100==cards[1]%100)
                    bt=pb_enum::BUNCH_AA;
            else if(cards[0]%100>=transformValue(14)&&cards[1]%100>=transformValue(14))
                // 2 Jokers
                bt=pb_enum::BUNCH_AAAA;
            break;
        case 3:
            if(cards[0]%100==cards[1]%100&&cards[0]%100==cards[2]%100)
                bt=pb_enum::BUNCH_AAA;
            break;
        case 4:
            if(cards[0]%100==cards[1]%100&&cards[0]%100==cards[2]%100
               &&cards[0]%100==cards[3]%100)
                bt=pb_enum::BUNCH_AAAA;
            /*
            else if(cards[0]%100==cards[1]%100&&cards[2]%100==cards[3]%100
               &&cards[0]%100+1==cards[2]%100)
                bt=pb_enum::BUNCH_ABC;  //AABB
            */
            else{
                //collect all counts
                std::map<uint32,int> valCount;  //[value,count]
                int maxSame=0;
                for(auto card:cards){
                    auto val=card%100;
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
                auto val=card%100;
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
                        if(cards[i]%100!=cards[i+1]%100)
                            bt=pb_enum::BUNCH_INVALID;
                        else if(i+2<len&&cards[i]%100+1!=cards[i+2]%100)
                            bt=pb_enum::BUNCH_INVALID;
                        i+=2;
                    }
                    break;
                case 1:
                    bt=pb_enum::BUNCH_ABC;
                    for(size_t i=0;i<len-1;++i){
                        if(cards[i]%100+1!=cards[i+1]%100){
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
    cards2str(str,bunch.pawns());
    //KEYE_LOG("verifyBunch pos=%d,type=%d: %s\n",bunch.pos(),bunch.type(),str.c_str());
    return bt;
}

bool DoudeZhu::validId(uint id){
    auto color=id/1000;
    if(color<1||color>4)return false;
    auto value=id%100;
    if(value<1||value>transformValue(15))return false;
    return true;
}

bool DoudeZhu::comparision(uint x,uint y){
    return x%100<y%100;
}

bool DoudeZhu::compareBunch(bunch_t& bunch,bunch_t& hist){
    //rule win
    auto win=false;
    auto bt=bunch.type();
    if(bt==pb_enum::BUNCH_INVALID){
        win=false;
    }else if(bt==pb_enum::BUNCH_AAAA){
        //bomb
        auto histCard=hist.pawns(0);
        auto bunchCard=bunch.pawns(0);
        if(hist.type()!=pb_enum::BUNCH_AAAA||histCard%100<bunchCard%100)
            win=true;
    }else if(bt==hist.type()&&bunch.pawns().size()==hist.pawns().size()){
        //same type and length
        switch (bt) {
            case pb_enum::BUNCH_AAAB:
            case pb_enum::BUNCH_AAAAB:{
                std::vector<unit_id_t> bunchCards,histCards;
                for(auto c:bunch.pawns())bunchCards.push_back(c);
                for(auto c:hist.pawns())histCards.push_back(c);
                //find value of the same cards
                uint32 bunchVal=0,histVal=0;
                if(pb_enum::BUNCH_AAAB==bt){
                    for(size_t i=0,ii=bunchCards.size()-2;i<ii;++i){
                        if(bunchCards[i]%100==bunchCards[i+1]%100&&bunchCards[i]%100==bunchCards[i+2]%100)
                            bunchVal=bunchCards[i]%100;
                    }
                    for(size_t i=0,ii=histCards.size()-2;i<ii;++i){
                        if(histCards[i]%100==histCards[i+1]%100&&histCards[i]%100==histCards[i+2]%100)
                            histVal=histCards[i]%100;
                    }
                }else{
                    for(size_t i=0,ii=bunchCards.size()-3;i<ii;++i){
                        if(bunchCards[i]%100==bunchCards[i+1]%100&&bunchCards[i]%100==bunchCards[i+2]%100
                           &&bunchCards[i]%100==bunchCards[i+3]%100)
                            bunchVal=bunchCards[i]%100;
                    }
                    for(size_t i=0,ii=histCards.size()-3;i<ii;++i){
                        if(histCards[i]%100==histCards[i+1]%100&&histCards[i]%100==histCards[i+2]%100
                           &&histCards[i]%100==histCards[i+3]%100)
                            histVal=histCards[i]%100;
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
                auto histCard=hist.pawns(0);
                auto bunchCard=bunch.pawns(0);
                if(histCard%100<bunchCard%100)
                    win=true;
                break;
            }
        }
    }
    std::string str,str1;
    cards2str(str,bunch.pawns());
    cards2str(str1,hist.pawns());
    //KEYE_LOG("compare win=%d [pos=%d,type=%d: %s] [pos=%d,type=%d: %s]\n",win,bunch.pos(),bunch.type(),str.c_str(),hist.pos(),hist.type(),str1.c_str());
    return win;
}

void DoudeZhu::make_bunch(proto3::bunch_t& bunch,const std::vector<uint>& vals){
    bunch.mutable_pawns()->Clear();
    for(auto id:vals){
        bunch.mutable_pawns()->Add(id);
    }
}

void DoudeZhu::test(){
    DoudeZhu ddz;
    Game game;
    ddz.deal(game);
    proto3::bunch_t A,B;
    A.set_pos(0);
    B.set_pos(1);
    std::vector<uint> va{5,6,7,8,9};
    std::vector<uint> vb{4,5,6,7,8};
    ddz.make_bunch(A,va);
    ddz.make_bunch(B,vb);
    
    ddz.verifyBunch(A);
    ddz.verifyBunch(B);
    ddz.compareBunch(A,B);
}


