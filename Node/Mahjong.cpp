//
//  Mahjong.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

int Mahjong::Type(){
    return pb_enum::GAME_MJ;
}

int Mahjong::MaxPlayer(Game& game){
    return 4;
}

int Mahjong::maxCards(Game& game){
    switch (game.category) {
        case pb_enum::MJ_SICHUAN:
            return 108;
        case pb_enum::MJ_HUNAN:
            return 108;
        case pb_enum::MJ_GUANGDONG:
            return 136;
        default:
            break;
    }
    return 108;
}

int Mahjong::maxHands(Game& game){
    return 13;
}

int Mahjong::bottom(Game& game){
    return 1;
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
    switch (game.category) {
        case pb_enum::MJ_GUANGDONG:
        case pb_enum::MJ_FUJIAN:
            //East,South,West,North
            for(int i=1;i<=4;++i){      //1-4
                for(int k=0;k<4;++k){   //xxxx
                    unit_id_t id=4000+k*100+i;
                    game.pile.push_back(id);
                }
            }
            //Zhong,Fa,Bai
            for(int i=1;i<=3;++i){      //1-3
                for(int k=0;k<4;++k){   //xxxx
                    unit_id_t id=5000+k*100+i;
                    game.pile.push_back(id);
                }
            }
            break;
        case pb_enum::MJ_SICHUAN:
        case pb_enum::MJ_HUNAN:
        default:
            break;
    }
    switch (game.category) {
        case pb_enum::MJ_FUJIAN:
            //Spring,Summer,...
            for(int i=1;i<=8;++i){      //1-8
                unit_id_t id=6000+i;
                game.pile.push_back(id);
            }
            break;
        default:
            break;
    }
}

void Mahjong::engage(Game& game,MsgNCEngage& msg){
    for(auto& p:game.players)msg.set_keys(p->playData.seat(),p->playData.selected_card());
    MeldGame::engage(game,msg);
}

bool Mahjong::meld(Game& game,Player& player,unit_id_t card,proto3::bunch_t& bunch){
    //could AAAA anywhere with draw
    std::vector<const bunch_t*> v;
    //unpack children
    v.push_back(&bunch);
    for(auto& ch:bunch.child())v.push_back(&ch);
    
    //meld all
    for(auto bun:v){
        auto ret=bun->type();
        std::vector<const bunch_t*> meldBunch;

        if(ret==pb_enum::BUNCH_A){
            //collect after draw
            player.playData.mutable_hands()->Add(card);
            //remove from pile map
            game.pileMap.erase(card);
            //draw with AAAA
            for(auto b:bun->child()){
                meldBunch.push_back(&b);
                
                //replay
                auto op=game.spReplay->add_ops();
                op->CopyFrom(b);
                op->set_type(BUNCH_A);
            }
        }else{
            meldBunch.push_back(bun);
            
            //replay
            auto op=game.spReplay->add_ops();
            op->CopyFrom(*bun);
        }
        
        for(auto b:meldBunch){
            //erase from hands
            auto& hands=*player.playData.mutable_hands();
            for(auto j:b->pawns()){
                for(auto i=hands.begin();i!=hands.end();++i){
                    if(j==*i){
                        //Debug<<"OnMeld pos=%d,erase card %d\n",where,*i);
                        hands.erase(i);
                        break;
                    }
                }
            }
            //then meld
            auto h=player.playData.add_bunch();
            h->CopyFrom(*b);
        }
    }
    return true;
}

void Mahjong::onMeld(Game& game,Player& player,unit_id_t card,proto3::bunch_t& bunch){
    if(bunch.type()==pb_enum::BUNCH_AAAA){
        //fix position for self draw
        changePos(game,(game.token+MaxPlayer(game)-1)%MaxPlayer(game));
    }
}

bool Mahjong::isWin(Game& game,proto3::bunch_t& bunch,std::vector<proto3::bunch_t>& output){
    if(bunch.type()<pb_enum::BUNCH_WIN){
        Debug<<"isWin failed: wrong bunch type\n";
        return false;
    }

    //log
    /*
    Debug<<"win bunches: "<<endl;
    for(auto& b:bunch.child()){
        std::string strbun;
        Debug<<bunch2str(strbun,b)<<endl;
    }
    */

    auto pos=bunch.pos();
    auto card=bunch.pawns(0);
    auto& player=*game.players[pos];
    auto& hands=player.playData.hands();
    auto& suite=*player.playData.mutable_bunch();

    if(hands.size()<1){
        Debug<<"isWin failed: len="<<hands.size()<<endl;
        return false;
    }
    
    //build a hand cards map
    std::map<unit_id_t,int> cmap;
    for(auto& a4:player.AAAAs)for(auto c:a4.pawns())cmap[c]=1;
    for(auto& a4:player.AAAs)for(auto c:a4.pawns())cmap[c]=1;
    for(auto& a4:suite)for(auto c:a4.pawns())cmap[c]=1;
    for(auto c:hands)cmap[c]=1;
    
    //check cards exists
    for(auto& b:bunch.child()){
        for(auto c:b.pawns()){
            if(cmap.find(c)!=cmap.end())
                --cmap[c];
            else if(c!=card){
                Debug<<"isWin failed: card "<<c<<" not exists\n";
                return false;
            }
        }
    }
    for(auto& kv:cmap)if(kv.second!=0){
        Debug<<"isWin failed: card "<<kv.first<<" missing\n";
        return false;
    }
    
    //verify bunch
    for(auto& b:*bunch.mutable_child()){
        if(pb_enum::BUNCH_INVALID==verifyBunch(game,b)){
            std::string str;
            Debug<<"isWin failed: invalid bunch "<<bunch2str(str,b)<<endl;
            return false;
        }
    }

    std::copy(bunch.child().begin(),bunch.child().end(),std::back_inserter(output));
    return true;
}

void Mahjong::settle(Player& player,std::vector<proto3::bunch_t>& allSuites,unit_id_t card){
    auto pos=player.playData.seat();
    auto& game=*player.game;
    auto M=MaxPlayer(game);
    int score=1;
    for(int i=0;i<M;++i){
        auto& play=game.players[i]->playData;
        if(pos==i&&allSuites.size()>0){
            play.set_win(1);
            play.set_score((M-1)*score);
        }else{
            play.set_score(-score);
        }
    }
}

void Mahjong::sortPendingMeld(std::shared_ptr<Game> spgame,std::vector<proto3::bunch_t>& pending){
    MeldGame::sortPendingMeld(spgame,pending);
    for(auto i=spgame->pendingMeld.begin()+1,iend=spgame->pendingMeld.end();i!=iend;++i){
        if(i->bunch.type()>=pb_enum::BUNCH_WIN)
            pending.push_back(i->bunch);
    }
}

void Mahjong::calcAchievement(Player& player,const std::vector<bunch_t>& suites,std::vector<achv_t>& avs){
    Game& game=*player.game;
    //统计工作
    int red=0,big=0,small=0;
    auto last=false;
    std::map<int,int> redmap;redmap[2]=0;redmap[7]=0;redmap[10]=0;
    for(auto i=suites.begin(),ii=suites.end(); i!=ii; ++i){
        for(auto j:i->pawns()){
            //红牌
            auto A=j;
            auto v=A%100;
            if(v==2||v==7||v==10){
                ++red;
                ++redmap[v];
            }
            //大小牌
            if(A/1000==1)++small;
            else ++big;
            //海底牌
            if(game.lastCard==j)last=true;
        }
        //对子
//        int ops=i->type();	ops=fixOps((pb_enum)ops);
//        if(pair&&(ops==pb_enum::PHZ_ABC||ops==pb_enum::UNKNOWN))pair=false;
    }
    
    //海胡
    pb_enum rule=game.category;
    if(last){
        if(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_GX){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_LAST);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
//            ach.set_value(nnn[ach.type()][rule]);
        }
    }
}

pb_enum Mahjong::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_INVALID;
    switch (bunch.type()) {
        case pb_enum::BUNCH_A:
            if(bunch.pawns_size()==1)
                bt=bunch.type();
            break;
        case pb_enum::BUNCH_AA:
            if(bunch.pawns_size()==2){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                if(A/1000==B/1000 && A%100==B%100)
                    bt=bunch.type();
            }
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
        case pb_enum::BUNCH_ABC:
            if(bunch.pawns_size()==3){
                std::vector<unit_id_t> cards(bunch.pawns().begin(),bunch.pawns().end());
                auto sorter=std::bind(&Mahjong::comparision,this,std::placeholders::_1,std::placeholders::_2);
                std::sort(cards.begin(),cards.end(),sorter);
                
                auto A=cards[0];
                auto B=cards[1];
                auto C=cards[2];
                if(A/1000==B/1000&&A/1000==C/1000&&
                   A%100+1==B%100&&A%100+2==C%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::OP_PASS:
        case pb_enum::BUNCH_WIN:
            bt=bunch.type();
        default:
            break;
    }
    bunch.set_type(bt);
    return bt;
}

bool Mahjong::checkDiscard(Player& player,unit_id_t drawCard){
    auto sz=player.playData.hands_size();
    if(sz<=0)
        return false;
    
    auto ret=(sz%3==2);
    if(ret){
        MeldGame::checkDiscard(player,drawCard);
        player.game->pendingDiscard->bunch.add_pawns(drawCard);
    }
    
    return ret;
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
            if(card/1000==B/1000){
                Debug<<"huazhu("<<B<<") found when verify "<<A<<endl;
                return false;
            }
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


