//
//  DoudeZhu.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include <algorithm>
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

int DoudeZhu::Type(){
    return pb_enum::GAME_DDZ;
}

int DoudeZhu::MaxPlayer(Game& game){
    return 3;
}

int DoudeZhu::maxCards(Game& game){
    return 54;
}

int DoudeZhu::maxHands(Game& game){
    return 17;
}

int DoudeZhu::bottom(Game& game){
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

void DoudeZhu::engage(Game& game,MsgNCEngage& msg){
    DiscardGame::engage(game,msg);
}

void DoudeZhu::settle(Player& player){
    Game& game=*player.game;
    game.spSettle=std::make_shared<MsgNCSettle>();
    auto& msg=*game.spSettle;
    for(uint i=0,ii=MaxPlayer(game);i!=ii;++i){
        auto play=msg.add_play();
        play->set_win(i==player.pos?1:0);
        play->mutable_hands()->CopyFrom(game.players[i]->playData.hands());
        //auto player=msg.add_play();
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
        case 0:
            bt=pb_enum::OP_PASS;
            break;
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


