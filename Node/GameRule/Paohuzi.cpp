//
//  Paohuzi.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

pb_enum fixOps(pb_enum ops){
    if(ops>=pb_enum::BUNCH_WIN)
        ops=(pb_enum)(ops%pb_enum::BUNCH_WIN);
    return ops;
}

int Paohuzi::Type(){
    return pb_enum::GAME_PHZ;
}

int Paohuzi::MaxPlayer(){
    return 3;
}

int Paohuzi::maxCards(){
    return 80;
}

int Paohuzi::maxHands(){
    return 20;
}

int Paohuzi::bottom(){
    return 1;
}

void Paohuzi::initCard(Game& game){
    //id: [color-index-value]
    for(int j=1;j<=2;++j){          //Big,Small => 1-2
        for(int i=1;i<=10;++i){      //1-10
            for(int k=0;k<4;++k){   //xxxx
                unit_id_t id=j*1000+k*100+i;
                game.pile.push_back(id);
            }
        }
    }
}

void Paohuzi::meld(Game& game,pos_t pos,unit_id_t card,bunch_t& bunch){
    auto& player=*game.players[pos];
    //erase from hands
    auto& hands=*player.playData.mutable_hands();
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
    auto h=player.playData.add_bunch();
    h->CopyFrom(bunch);
    //pending discard
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->bunch.set_pos(pos);
    changePos(game,pos);
}

bool Paohuzi::isGameOver(Game& game,pos_t pos,unit_id_t card,std::vector<bunch_t>& output){
    auto player=game.players[pos];
    auto& playdata=game.players[pos]->playData;
    auto& suite=*playdata.mutable_bunch();
    auto& hands=player->playData.hands();
    if(hands.size()<2){
        KEYE_LOG("isGameOver failed: len=%d\n",hands.size());
        return false;
    }
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));
    cards.push_back(card);
    auto sorter=std::bind(&Paohuzi::comparision,this,std::placeholders::_1,std::placeholders::_2);
    std::sort(cards.begin(),cards.end(),sorter);
    
    //check desk
    auto bDraw=game.pileMap.find(card)!=game.pileMap.end();
    auto myself=(pos==game.token);
    
    std::vector<bunch_t> allSuites(suite.begin(),suite.end());
    std::vector<unit_id_t> hand(hands.begin(),hands.end());
    
    logHands(game,pos);
    //是否需要将
    bool needJiang=false;
    for(auto iv=suite.begin(),ivv=suite.end(); iv!=ivv; ++iv){
        auto ops=fixOps(iv->type());
        if(ops==pb_enum::PHZ_AAAA||ops==pb_enum::PHZ_AAAAstart||ops==pb_enum::PHZ_BBB_B||ops==pb_enum::PHZ_BBBBdesk||ops==PHZ_AAAAdesk){
            needJiang=true;
            break;
        }
    }
    
    //按颜色和点数排序
    std::vector<std::vector<int>> mc[2];
    mc[0].resize(10);
    mc[1].resize(10);
    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it){
        auto C=*it;
        int x=C/1000-1;
        auto& v=mc[x][C%100-1];
        v.push_back(C);
        //可能刚跑，仍检查手牌
        if(v.size()>=4)needJiang=true;
    }
    
    //坎牌剔除，记录所有将
    std::vector<bunch_t> jiangs;
    for(int j=0; j<2; ++j){
        auto& v=mc[j];
        for(auto iv=v.begin(),ivv=v.end(); iv!=ivv; ++iv){
            if(needJiang&&iv->size()==2){
                //add to suite
                jiangs.push_back(bunch_t());
                auto &su=jiangs.back();
                su.set_type(pb_enum::BUNCH_AA);
                for(auto it=iv->begin(),iend=iv->end();it!=iend;++it)su.add_pawns(*it);
            } else if(iv->size()>=3){
                //处理3cards中不是坎的情况
                bool btmp=false;
                for(auto x=iv->begin(),xx=iv->end(); x!=xx; ++x){
                    if((bDraw||!myself)&&card==*x&&iv->size()==3){
                        btmp=true;
                        break;
                    }
                }
                if(btmp){
                    //allSuites.pop_back();
                    std::vector<int> tmpCards(*iv);
                    jiangs.push_back(bunch_t());
                    auto &su=jiangs.back();
                    su.set_type(pb_enum::PHZ_AA);
                    tmpCards.pop_back();
                    for(auto it=tmpCards.begin(),iend=tmpCards.end();it!=iend;++it)su.add_pawns(*it);
                    break;
                }
                //处理坎牌
                //add to stratagy suite
                allSuites.push_back(bunch_t());
                auto &su=allSuites.back();
                auto bAAA=true;
                for(auto x=iv->begin(),xx=iv->end(); x!=xx; ++x){
                    if(card==*x)bAAA=false;
                    su.add_pawns(*x);
                    //remove from hands
                    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it)
                        if(*x==*it){
                            hand.erase(it);
                            break;
                        }
                }
                if(iv->size()==3)
                    su.set_type((bAAA?pb_enum::PHZ_AAA:(bDraw&&myself?pb_enum::PHZ_AAAwei:pb_enum::PHZ_BBB)));
                else
                    su.set_type((bDraw&&myself?pb_enum::PHZ_AAAA:pb_enum::PHZ_BBB_B));
            }
        }
    }
    //提坎已剔除，只剩绞，句
    bool over=false;
    if(needJiang){
        //优先剔除将
        if(jiangs.size()<1)return false;
        int PT=0;
        bool btmpOver=false;
        std::vector<bunch_t> resSuites;
        auto tmpJiang=jiangs.end();
        for(auto ij=jiangs.begin(),ijj=jiangs.end(); ij!=ijj; ++ij){
            std::vector<bunch_t> tmpSuites(allSuites);
            auto& jiang=*ij;
            //make a temp hand cards
            std::vector<unit_id_t> tmpHand;
            for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it)
                if(jiang.pawns(0)!=*it&&jiang.pawns(1)!=*it)
                    tmpHand.push_back(*it);
            //recursived
            //over=isGameOver(tmpHand,allSuites);
            over=isGameOver(game,tmpHand,tmpSuites);
            if(over){
                auto pt=calcPoints(game,tmpSuites);
                if(pt>=PT){
                    resSuites.clear();
                    PT=pt;
                    btmpOver=over;
                    tmpJiang=ij;
                    std::copy(tmpSuites.begin(),tmpSuites.end(),std::back_inserter(resSuites));
                }
            }
        }
        over=btmpOver;
        if(btmpOver){
            resSuites.push_back(*tmpJiang);
            allSuites.clear();
            std::copy(resSuites.begin(),resSuites.end(),std::back_inserter(allSuites));
        }else{
            return false;
        }
    } else{
        //recursived
        over=isGameOver(game,hand,allSuites);
    }
    if(over){
        auto M=MaxPlayer();
        int MIN_SUITES=7;
        if(M==4)MIN_SUITES=5;//衡阳，碰胡子玩法
        if(allSuites.size()>=MIN_SUITES){
            if(game.category==pb_enum::PHZ_PEGHZ){
                //碰胡子判胡
                std::copy(allSuites.begin(),allSuites.end(),std::back_inserter(output));
                return true;
            }
            if(auto point=calcPoints(game,allSuites))
                if(point>=winPoint(game,game.category)){
                    std::copy(allSuites.begin(),allSuites.end(),std::back_inserter(output));
                    return true;
                }
        }
    }
    
    return false;
}

bool Paohuzi::isGameOver(Game& game,std::vector<unit_id_t>& cards,std::vector<bunch_t>& allSuites){
    //recursive check if is game over
    std::vector<bunch_t> outSuites;
    std::vector<bunch_t> multiSuites;
    //copy hand
    std::vector<unit_id_t> _cards(cards);
    
    //logCards(_cards,"-----isGameOver:");
    //先找每张牌的组合，如果没有则返回
    for(auto i=cards.begin(),ii=cards.end(); i!=ii; ++i){
        auto card=*i;
        std::vector<bunch_t> hints;
        hint(game,card,_cards,hints);
        if(hints.empty()){
            //此牌无组合
            //log("isGameOver no suite for card=%d:%d",card,allCards[card]%100);
            return false;
        } else if(hints.size()==1){
            //此牌唯一组合,剔除
            //log("isGameOver single suite for card=%d:%d", card, allCards[card]%100);
            auto& ihint=hints.front();
            outSuites.push_back(ihint);
            multiSuites.clear();
            for(auto k=_cards.begin(); k!=_cards.end();){
                auto found=false;
                for(auto j=ihint.pawns().begin(); j!=ihint.pawns().end(); ++j)
                    if(*k==*j){
                        found=true;
                        break;
                    }
                if(found){
                    k=_cards.erase(k);
                }
                else ++k;
            }
            //递归
            if(isGameOver(game,_cards,outSuites)){
                std::copy(outSuites.begin(),outSuites.end(),std::back_inserter(allSuites));
                return true;
            } else
                return false;
        } else{
            std::copy(hints.begin(),hints.end(),std::back_inserter(multiSuites));
        }
    }
    
    //存在多张组合的牌
    std::vector<std::vector<bunch_t>> vvm;
    if(!multiSuites.empty()){
        //log("isGameOver cards=%d, multiSuites=%d", _cards.size(), multiSuites.size());
        //copy temp hand
        std::vector<bunch_t> tmphints;
        
        //遍历每个组合
        std::vector<bunch_t> vm;
        for(auto m=multiSuites.begin(),mm=multiSuites.end();m!=mm;++m){
            std::vector<unit_id_t> mcards(_cards);
            //此组合的牌从临时手牌移除，并加入临时suites
            for(auto l=m->pawns().begin(),ll=m->pawns().end();l!=ll;++l)
                for(auto k=mcards.begin(); k!=mcards.end();++k){
                    if(*k==*l){
                        mcards.erase(k);
                        break;
                    }
                }
            vm.clear();
            if(isGameOver(game,mcards,vm)){
                vm.push_back(*m);
                vvm.push_back(vm);
            }
        }
        if(!vvm.empty()){
            int PT=0;
            auto iwin=vvm.end();
            for(auto ivvm=vvm.begin(),ivvend=vvm.end();ivvm!=ivvend;++ivvm){
                auto pt=calcPoints(game,*ivvm);
                if(pt>=PT){
                    PT=pt;
                    iwin=ivvm;
                }
            }
            if(iwin!=vvm.end()){
                //最优胡牌组合
                std::copy(outSuites.begin(),outSuites.end(),std::back_inserter(allSuites));
                std::copy(iwin->begin(),iwin->end(),std::back_inserter(allSuites));
                return true;
            }
        }
        return false;
    }
    
    //all cards past
    return true;
}

bool Paohuzi::hint3(Game& game,pos_t pos,unit_id_t card,bunch_t& hints){
    //all possible AAA,AAAA,BBB,BBBB like
    auto& playdata=game.players[pos]->playData;
    auto& hand=*playdata.mutable_hands();
    auto& suite=*playdata.mutable_bunch();

    auto A=card;
    std::vector<int> tmp;
    
    //check desk
    auto bDraw=game.pileMap.find(A)!=game.pileMap.end();
    auto myself=(pos==game.token);
    
    //skip self
    if(myself&&!bDraw)return false;
    
    //find same cards in hands
    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it){
        auto B=*it;
        if(A/1000==B/1000 && A%100==B%100)
            tmp.push_back(B);
    }
    
    if(tmp.size()>=2){
        for(auto it=tmp.begin(),iend=tmp.end();it!=iend;++it)hints.add_pawns(*it);
        hints.add_pawns(card);
        if(tmp.size()==2)
            hints.set_type(bDraw&&myself?pb_enum::PHZ_AAAwei:pb_enum::PHZ_BBB);
        else
            hints.set_type((bDraw&&myself?pb_enum::PHZ_AAAA:pb_enum::PHZ_BBB_B));
        //臭偎
        if(hints.type()==pb_enum::PHZ_AAAwei&&chouWei(game,pos,hints))
            hints.set_type(pb_enum::PHZ_AAAchou);
        return true;
    }
    
    for(auto i=suite.begin(),ii=suite.end(); i!=ii; ++i){
        //检测桌面牌组：偎子直接跑，碰子须抓的才能跑
        if(i->type()==pb_enum::PHZ_AAAwei||i->type()==pb_enum::PHZ_AAAchou||(i->type()==pb_enum::PHZ_BBB&&bDraw)){
            auto B=i->pawns(0);
            if(A%100==B%100 && A/1000==B/1000 && A!=B){
                for(auto c:i->pawns())hints.add_pawns(c);
                hints.add_pawns(card);
                hints.set_type((i->type()==pb_enum::PHZ_AAAwei && bDraw && myself?PHZ_AAAAdesk:pb_enum::PHZ_BBBBdesk));
                return true;
            }
        }
    }
    return false;
}

void Paohuzi::hint(Game& game,unit_id_t card,std::vector<unit_id_t>& _hand,std::vector<bunch_t>& hints){
    //这张牌可能的所有组合：句,绞
    std::vector<int> jiao,same;
    auto A=card;
    
    std::vector<unit_id_t> hand(_hand);
    //按颜色和点数排序
    std::vector<std::vector<int>> mc[2];
    mc[0].resize(10);
    mc[1].resize(10);
    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it){
        if(card==*it)continue;	//跳过自己
        auto C=*it;
        int x=C/1000-1;
        mc[x][C%100-1].push_back(C);
    }
    
    //坎牌剔除
    for(int j=0; j<2; ++j){
        auto& v=mc[j];
        for(auto iv=v.begin(),ivv=v.end(); iv!=ivv; ++iv){
            if(iv->size()==3){
                for(auto x=iv->begin(),xx=iv->end(); x!=xx; ++x){
                    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it)
                        if(*x==*it){
                            hand.erase(it);
                            break;
                        }
                }
            }
        }
    }
    
    //找相同点数牌
    for(auto it=hand.begin(),iend=hand.end();it!=iend;++it){
        if(card==*it)continue;	//跳过自己
        auto B=*it;
        if(B%100==A%100){
            if(B/1000!=A/1000)jiao.push_back(*it);
            else same.push_back(*it);
        }
    }
    if(jiao.size()==2){
        //绞，append to hints: Bbb
        hints.push_back(bunch_t());
        auto& suite=hints.back();
        suite.set_type(pb_enum::PHZ_AbA);
        for(auto c:jiao)suite.add_pawns(c);
        suite.add_pawns(card);
        //log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.pawns(0], suite.pawns(1], suite.pawns(2]);
    }
    if(same.size()>0&&jiao.size()>0){
        //绞，append to hints: BBb
        hints.push_back(bunch_t());
        auto& suite=hints.back();
        suite.set_type(pb_enum::PHZ_AbA);
        for(auto it=same.begin(),iend=same.end();it!=iend;++it)
            if(card!=*it){
                //别把自己放进去
                suite.add_pawns(*it);
                break;
            }
        suite.add_pawns(jiao.front());
        suite.add_pawns(card);
        //log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.pawns(0], suite.pawns(1], suite.pawns(2]);
    }
    
    //句 首先查找与A相临 前面两张牌和后面两张牌的id 和数量 -1表示存在
    std::vector<int> FontABackID[4];	// CBBC
    for (auto it=hand.begin(), iend=hand.end(); it!=iend; ++it) {
        auto B=*it;
        if( B/1000 != A/1000 ) {
            continue;	// 颜色不一样 直接跳过
        }
        int off = B%100 - A%100;
        if ( off == 0 ) {
            continue;
        }
        off += (off > 0 ? 1 : 2);		// 计算这个牌的坐标
        if ( off >= 0 && off <= 3 ) {
            FontABackID[off].push_back( *it );
        }
    }
    //
    for ( int i = 0; i < 3; ++i ) {
        hints.push_back(bunch_t());
        auto& suite=hints.back();
        suite.set_type(pb_enum::PHZ_ABC);
        suite.add_pawns(FontABackID[i][0]);
        suite.add_pawns(FontABackID[i+1][0]);
        suite.add_pawns(card);
    }
    
    //find same color and sort
    std::vector<int> color;
    for(auto it=hand.begin(),iend=hand.end(); it!=iend; ++it){
        if(card==*it)continue;	//跳过自己
        auto B=*it;
        if(B/1000==A/1000)color.push_back(*it);
    }
    //2,7,10
    std::map<int,std::vector<int>> mm;	//[value,ids]
    if(A%100==2||A%100==7||A%100==10){
        for(auto it=color.begin(),iend=color.end(); it!=iend;++it){
            auto B=*it;
            if(B%100!=A%100&&(B%100==2||B%100==7||B%100==10))
                mm[B%100].push_back(B);
        }
        if(mm.size()==2){
            auto& E=mm.begin()->second;
            auto& F=mm.rbegin()->second;

            hints.push_back(bunch_t());
            auto& suite=hints.back();
            suite.set_type(pb_enum::PHZ_ABC);
            suite.add_pawns(E[0]);
            suite.add_pawns(F[0]);
            suite.add_pawns(card);
            //log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.pawns(0], suite.pawns(1], suite.pawns(2]);
        }
    }
}

bool Paohuzi::hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,bunch_t& src_bunch){
    //for: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
   
    auto count=bunches.size();
    if(count>0){
        std::string str,ss;
        for(auto& bunch:bunches){
            bunch2str(ss,bunch);
            str+=ss;
        }
        KEYE_LOG("hint %d,pos=%d,%s\n",count,pos,str.c_str());
    }
    return count>0;
}

pb_enum Paohuzi::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_INVALID;
    auto type=fixOps(bunch.type());
    switch(type) {
        case pb_enum::PHZ_AAA:
        case pb_enum::PHZ_AAAwei:
        case pb_enum::PHZ_AAAchou:
        case pb_enum::PHZ_BBB:
            if(bunch.pawns_size()==3){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                auto C=bunch.pawns(2);
                if(A/1000==B/1000 && A/1000==C/1000 &&
                   A%100==B%100 && A%100==C%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::PHZ_AAAA:
        case pb_enum::PHZ_AAAAstart:
        case pb_enum::PHZ_AAAAdesk:
        case pb_enum::PHZ_BBB_B:
        case pb_enum::PHZ_BBBBdesk:
            if(bunch.pawns_size()==4){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                auto C=bunch.pawns(2);
                auto D=bunch.pawns(3);
                if(A/1000==B/1000 && A/1000==C/1000 && A/1000==D/1000 &&
                   A%100==B%100 && A%100==C%100 && A%100==D%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::PHZ_ABC:
            if(bunch.pawns_size()==3){
                std::vector<unit_id_t> cards;
                std::copy(bunch.pawns().begin(),bunch.pawns().end(),std::back_inserter(cards));
                auto sorter=std::bind(&Paohuzi::comparision,this,std::placeholders::_1,std::placeholders::_2);
                std::sort(cards.begin(),cards.end(),sorter);

                auto A=cards[0];
                auto B=cards[1];
                auto C=cards[2];
                if(A/1000==B/1000 && A/1000==C/1000 &&
                   ((A%100+1==B%100 && A%100+2==C%100) ||
                    (A%100==2 && B%100==7 && C%100==10)))
                    bt=bunch.type();
            }
            break;
        case pb_enum::PHZ_AbA:
            if(bunch.pawns_size()==3){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                auto C=bunch.pawns(2);
                if(A%100==B%100 && A%100==C%100 &&
                    !(A/1000==B/1000 && A/1000==C/1000))
                    bt=bunch.type();
            }
            break;
        case pb_enum::PHZ_AA:
            if(bunch.pawns_size()==2){
                auto A=bunch.pawns(0);
                auto B=bunch.pawns(1);
                if(A/1000==B/1000 && A%100==B%100)
                    bt=bunch.type();
            }
            break;
        case pb_enum::OP_PASS:
            bt=bunch.type();
            break;
        default:
            //invalid
            break;
    }
    bunch.set_type(bt);
    return bt;
}

bool Paohuzi::validId(uint id){
    auto color=id/1000;
    if(color<1||color>2)return false;
    auto value=id%100;
    if(value<1||value>10)return false;
    return true;
}

void Paohuzi::calcAchievement(Game& game,pb_enum rule,const std::vector<bunch_t>& suites,std::vector<achv_t>& avs){
    /*
    card_t::eAchievment archievment=card_t::eAchievment::WIN_NORMAL;
    auto& allCards=_desk->allCards;
    //统计工作
    int red=0,big=0,small=0;
    auto pair=true;
    auto last=false;
    std::map<int,int> redmap;redmap[2]=0;redmap[7]=0;redmap[10]=0;
    for(auto i=suites.begin(),ii=suites.end(); i!=ii; ++i){
        for(auto j=i->cards.begin(),jj=i->cards.end(); j!=jj; ++j){
            //红牌
            auto& A=allCards[*j];
            auto v=A.value;
            if(v==2||v==7||v==10){
                ++red;
                ++redmap[v];
            }
            //大小牌
            if(A.small)++small;
            else ++big;
            //海底牌
            if(_desk->_lastCard==*j)last=true;
        }
        //对子
        int ops=i->ops;	ops=fixOps((pb_enum)ops);
        if(pair&&(ops==pb_enum::AaA||ops==pb_enum::PHZ_ABC||ops==pb_enum::UNKNOWN))pair=false;
    }
    
    //海胡
    if(last){
        if(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_GX){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_LAST;
            ach.multiple=nnn[ach.type][rule];
        }
    }
    
    //红乌
    if(red>=10&&(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_XX_GHZ)){
        avs.push_back(AchievementData());
        auto& ach=avs.back();
        if(red>=13){
            ach.type=card_t::eAchievment::WIN_13RED;
            ach.points=nnn[ach.type][rule];
        } else{
            ach.type=card_t::eAchievment::WIN_RED;
            ach.multiple=nnn[ach.type][rule];
        }
    }
    if(red>=10&&(rule==pb_enum::PHZ_SY)){
        avs.push_back(AchievementData());
        auto& ach=avs.back();
        ach.type=card_t::eAchievment::WIN_RED;
        ach.multiple=nnn[ach.type][rule];
    }
    //红胡
    if(red>=10&&(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT)){
        avs.push_back(AchievementData());
        auto& ach=avs.back();
        ach.type=card_t::eAchievment::WIN_RED;
        ach.multiple=nnn[ach.type][rule];
        ach.multiple+=(red-10);
    }
    if(rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_HHD){
        if(red>=13&&rule!=pb_enum::PHZ_HH){
            //红乌
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_13RED;
            ach.multiple=nnn[ach.type][rule];
        }else if(red>=10){
            //红胡
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_RED;
            ach.multiple=nnn[ach.type][rule];
            if(rule==pb_enum::PHZ_HH)
                ach.multiple+=(red-10);
        }
    }
    if(red==2){
        //双飘
        if(rule==pb_enum::PHZ_CS){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_2RED_;
            ach.multiple=nnn[ach.type][rule];
        }
    } else if(red==1){
        //点胡
        if(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT||
           rule==pb_enum::PHZ_CD_HHD||rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_SINGLE;
            ach.multiple=nnn[ach.type][rule];
        }
    } else if(red==0){
        //黑胡
        if(rule==pb_enum::PHZ_SY||rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH||
           rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_CD_HHD||
           rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_XX_GHZ){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_BLACK;
            if(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_XX_GHZ)
                ach.points=nnn[ach.type][rule];
            else
                ach.multiple=nnn[ach.type][rule];
        }
    }
    //二三四比
    if(rule==pb_enum::PHZ_CS){
        if(red==2&&(redmap[2]==2&&redmap[7]==0&&redmap[10]==0||
                    redmap[2]==0&&redmap[7]==2&&redmap[10]==0||
                    redmap[2]==0&&redmap[7]==0&&redmap[10]==2)){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_2RED;
            ach.multiple=nnn[ach.type][rule];
        }else if(red==3&&(redmap[2]==3&&redmap[7]==0&&redmap[10]==0||
                          redmap[2]==0&&redmap[7]==3&&redmap[10]==0||
                          redmap[2]==0&&redmap[7]==0&&redmap[10]==3)){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_3RED;
            ach.multiple=nnn[ach.type][rule];
        }else if(red==4&&(redmap[2]==4&&redmap[7]==0&&redmap[10]==0||
                          redmap[2]==0&&redmap[7]==4&&redmap[10]==0||
                          redmap[2]==0&&redmap[7]==0&&redmap[10]==4)){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_4RED;
            ach.multiple=nnn[ach.type][rule];
        }
    }
    //一块匾
    int _count=0;
    for(auto ic=suites.begin(),icc=suites.end();ic!=icc;++ic){
        //考虑到可能最后是吃，碰，跑，提偎等赢的，应该首先把应的状态去掉在做下边的处理即可
        auto res=fixOps(ic->ops);
        if(res!=pb_enum::PHZ_ABC&&res!=pb_enum::AaA&&res!=pb_enum::PHZ_AAA&&res!=pb_enum::PHZ_AAAwei&&res!=pb_enum::PHZ_AAAchou&&res!=pb_enum::PHZ_BBB&&res!=pb_enum::PHZ_AAAA&&res!=pb_enum::PHZ_AAAAstart&&res!=pb_enum::PHZ_BBB_B&&res!=pb_enum::PHZ_AAAAdesk)
            continue;
        int count=0;
        for(auto iv=ic->cards.begin(),ivv=ic->cards.end();iv!=ivv;++iv){
            auto A=allCards[*iv];
            if(A.value==2||A.value==7||A.value==10){
                count++;
            }
        }
        if((count==3||count==4)&&(red==3||red==4)){
            _count++;
        }
    }
    if(_count==1&&rule==pb_enum::PHZ_LD){
        avs.push_back(AchievementData());
        auto& ach=avs.back();
        ach.type=card_t::eAchievment::WIN_PLATE;
        ach.multiple=nnn[ach.type][rule];
    }
    //if((red==3||red==4)&&rule==pb_enum::PHZ_LD){
    //if(redmap[2]==1&&redmap[7]==1&&redmap[10]==1||
    //(redmap[2] ==3||redmap[2] ==4)&&redmap[7]==0&&redmap[10]==0||
    //(redmap[7] ==3||redmap[7] ==4)&&redmap[2]==0&&redmap[10]==0||
    //(redmap[10]==3||redmap[10]==4)&&redmap[2]==0&&redmap[7] ==0){
    //进一步判断2,7,10是否是一个完整的组合，如果是则添加名堂，否则不添加名堂
    //avs.push_back(AchievementData());
    //auto& ach=avs.back();
    //ach.type=card_t::eAchievment::WIN_PLATE;
    //ach.multiple=nnn[ach.type][rule];
    //}
    //}
    //大小胡
    if(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_HH){
        if(big>=18){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_BIG;
            ach.multiple=nnn[ach.type][rule];
            ach.multiple+=big-18;
        }
        int S=(rule==pb_enum::PHZ_CS?18:16);
        if(small>=S){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_SMALL;
            ach.multiple=nnn[ach.type][rule];
            ach.multiple+=small-S;
        }
    }
    //对胡
    if(pair){
        if(rule==pb_enum::PHZ_CS||
           rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_QMT){
            avs.push_back(AchievementData());
            auto& ach=avs.back();
            ach.type=card_t::eAchievment::WIN_PAIR;
            ach.multiple=nnn[ach.type][rule];
        }
    }
    */
}

int Paohuzi::winPoint(Game&,pb_enum rule){
    int point=10;
    switch(rule){
        case pb_enum::PHZ_LD:		//娄底放炮
        case pb_enum::PHZ_HH:		//怀化红拐弯
        case pb_enum::PHZ_CD_QMT:	//常德全名堂
        case pb_enum::PHZ_CD_HHD:	//常德红黑点
        case pb_enum::PHZ_CS:		//长沙跑胡子
        case pb_enum::PHZ_XX_GHZ:	//湘乡告胡子
            point=15;	break;
        case pb_enum::PHZ_CZ:		//郴州毛胡子
            point=9;	break;
        case pb_enum::PHZ_HY:		//衡阳六条枪
            point=6;	break;
        case pb_enum::PHZ_GX:		//广西
            point=10;	break;
        case pb_enum::PHZ_SY:		//邵阳字牌
        case pb_enum::PHZ_SYBP:     //邵阳剥皮
        default:
            break;
    }
    return point;
}

int Paohuzi::calcScore(Game&,pb_enum rule,int points){
    int score=0;
    switch(rule){
        case pb_enum::PHZ_HH:		//怀化红拐弯
        case pb_enum::PHZ_CD_QMT:	//常德全名堂
        case pb_enum::PHZ_CD_HHD:	//常德红黑点
        case pb_enum::PHZ_CS:		//长沙跑胡子
            score=(points-12)/3;	//(x-15)/3+1
            break;
        case pb_enum::PHZ_SY:		//邵阳字牌
            score=(points-5)/5;		//(x-10)/5+1
            if(pb_enum::PHZ_SY==rule&&score>0)
                ++score;
            break;
        case pb_enum::PHZ_CZ: //郴州毛胡子
            score=(points-6)/3;
            break;
        case pb_enum::PHZ_HY:		//衡阳六条枪
            score=(points-3)/3;
            break;
        case pb_enum::PHZ_GX:		//广西
            score=(points-5)/5;
            break;
        case pb_enum::PHZ_LD:		//娄底放炮
        case pb_enum::PHZ_SYBP:	//邵阳剥皮
        case pb_enum::PHZ_XX_GHZ: //湘乡告胡子
        default:
            score=points;
            break;
    }
    if(score<0)score=0;
    return score;
}

int Paohuzi::calcPoints(Game&,std::vector<bunch_t>& allSuites){
    int point=0;
    for(auto i=allSuites.begin(),ii=allSuites.end(); i!=ii; ++i){
        auto& suite=*i;
        if(suite.pawns().empty())continue;
        auto small=suite.pawns(0)/1000;
        int pt=0;
        switch(fixOps(suite.type())){
            case pb_enum::PHZ_AAAA:
            case pb_enum::PHZ_AAAAstart:
            case pb_enum::PHZ_AAAAdesk:
                pt+=(small?9:12);
                break;
            case pb_enum::PHZ_BBBBdesk:
            case pb_enum::PHZ_BBB_B:
                pt+=(small?6:9);
                break;
            case pb_enum::PHZ_AAAwei:
            case pb_enum::PHZ_AAA:
            case pb_enum::PHZ_AAAchou:
                pt+=(small?3:6);
                break;
            case pb_enum::PHZ_BBB:
                pt+=(small?1:3);
                break;
            case pb_enum::PHZ_ABC:{
                std::vector<unit_id_t> sl(suite.pawns().begin(),suite.pawns().end());
                std::sort(sl.begin(),sl.end());
                auto A=sl[0];
                auto B=sl[1];
                if(A%100==1 || (A%100==2&&B%100==7))
                    pt+=(small?3:6);
                break;
            }
            default:
                break;
        }
        point+=pt;
        //log("settle point=%d, small=%d, ops=%s", pt, small, ops2String(suite.ops).c_str());
    }
    return point;
}

bool Paohuzi::chouWei(Game& game,pos_t pos,bunch_t& bunch){
    auto player=game.players[pos];
    //臭偎
    auto& vp=player->unpairedCards;
    auto n=bunch.pawns(0);
    for(auto i=vp.begin(),ii=vp.end(); i!=ii; ++i){
        auto m=*i;
        if(m%100==n%100&&m/1000==n/1000){
            //vp.erase(i);
            return true;
        }
    }
    return false;
}

void Paohuzi::test(){
    Paohuzi ddz;
    Game game;
    ddz.deal(game);
    bunch_t A,B;
    A.set_pos(0);
    B.set_pos(1);
    std::vector<uint> va{5,6,7,8,9};
    std::vector<uint> vb{4,5,6,7,8};
    ddz.make_bunch(A,va);
    ddz.make_bunch(B,vb);
    
    ddz.verifyBunch(game,A);
    ddz.verifyBunch(game,B);
}


