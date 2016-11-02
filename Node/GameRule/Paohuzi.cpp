//
//  Paohuzi.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
#include <sstream>
#include <algorithm>
#include <cmath>
using namespace proto3;

int nnn[pb_enum::WIN_MAX][pb_enum::PHZ_MAX]={
    //SY,SYBP,LD,   HH,CD_QMT,CD_HHD, CS,XX_GHZ,HY,  YZ_SBW,PEGHZ,SC_EQS,CZ,    GX
    {0,0,0,				0,0,0,			0,0,0,			0,0,0,0,				0},		//平胡
    {0,10,100,			4,6,0,			5,110,0,		0,20,0,2,				2},		//天胡
    {0,10,100,			3,6,0,			5,100,0,		0,16,0,2,				2},		//地胡
    {0,0,1,				2,6,0,			0,0,0,			0,0,0,0,				2},		//海胡
    {0,0,0,				0,6,0,			0,0,0,			0,0,0,0,				0},		//听胡
    
    {0,0,100,			5,0,4,			0,100,0,		0,0,0,0,				0},		//红乌
    {2,0,1,				2,3,2,			2,2,0,			0,0,0,0,				0},		//红胡
    {2,0,100,			5,8,5,			5,100,0,		0,0,0,0,				0},		//黑胡
    {0,0,0,				4,8,0,			5,0,0,			0,0,0,0,				0},		//大胡
    {0,0,0,				4,10,0,			5,0,0,			0,0,0,0,				0},		//小胡
    
    {0,0,1,				3,6,3,			4,0,0,			0,0,0,0,				0},		//点胡
    {0,0,0,				4,8,0,			5,0,0,			0,0,0,0,				0},		//对胡
    {0,0,0,				0,8,0,			0,0,0,			0,0,0,0,				0},		//耍猴
    {0,0,0,				0,2,0,			0,0,0,			0,0,0,0,				0},		//黄番
    {0,10,1,			1,1,0,			0,10,2,			0,0,0,2,				2},		//自摸
    
    {0,0,100,			0,0,0,			0,2,0,			0,0,0,0,				0},		//30胡
    {0,0,1,				0,0,0,			0,0,0,			0,0,0,0,				0},		//20胡
    {0,0,-1,			0,0,0,			0,10,-1,		0,0,0,3,				0},		//放炮
    {0,0,1,				0,0,0,			0,0,0,			0,0,0,0,				0},		//一块匾
    {0,0,0,				0,0,0,			2,0,0,			0,0,0,0,				0},		//二比
    
    {0,0,0,				0,0,0,			3,0,0,			0,0,0,0,				0},		//三比
    {0,0,0,				0,0,0,			4,0,0,			0,0,0,0,				0},		//四比
    {0,0,0,				0,0,0,			2,0,0,			0,0,0,0,				0},		//双飘
    {0,0,0,				0,0,0,			0,100,0,		0,0,0,0,				0},		//十红，湘乡
    
    {0,0,0,				0,0,0,			0,0,0,			0,40,0,0,				0},		//五福
    {0,0,0,				0,0,0,			0,0,0,			0,8,0,0,				0},		//跑双
    {0,0,0,				0,0,0,			0,0,0,			0,40,0,0,				0},		//小七对
    {0,0,0,				0,0,0,			0,0,0,			0,40,0,0,				0},		//双龙
    {0,0,0,				0,0,0,			0,0,0,			0,2,0,0,				0},		//连胡
};

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

void Paohuzi::engage(Game& game,MsgNCEngage& msg){
    //pick out AAAA,AAA;these process should be after engage,not at start time
    for(auto p:game.players){
        auto& hands=*p->playData.mutable_hands();

        //category hand card by color and value
        std::vector<std::vector<unit_id_t>> mc[2];
        mc[0].resize(10);
        mc[1].resize(10);
        for(auto C:hands){
            int x=(C/1000==2?1:0);
            mc[x][C%100-1].push_back(C);
        }
        //check cards more than 4 with same value and color
        for(int j=0;j<2;++j){
            auto& v=mc[j];
            for(auto iv=v.begin(),ivv=v.end();iv!=ivv;++iv){
                if(iv->size()>=3){
                    bunch_t* pb=nullptr;
                    if(iv->size()==4){
                        //add to AAAA
                        p->AAAAs.push_back(bunch_t());
                        pb=&p->AAAAs.back();
                        pb->set_type(pb_enum::PHZ_AAAAstart);
                    }else if(iv->size()==3){
                        //add to AAA
                        p->AAAs.push_back(bunch_t());
                        pb=&p->AAAs.back();
                        pb->set_type(pb_enum::PHZ_AAA);
                    }
                    //the cards
                    for(auto x=iv->begin(),xx=iv->end();x!=xx;++x){
                        pb->add_pawns(*x);
                        //remove from hands
                        for(auto it=hands.begin(),iend=hands.end(); it!=iend; ++it)
                            if(*x==*it){
                                hands.erase(it);
                                break;
                            }
                    }//for
                }//>3
            }//for iv
        }//for j
    }//for game players
    
    //packed AAAAs into message
    for(auto p:game.players){
        if(!p->AAAAs.empty()){
            auto bunch=msg.mutable_bunch(p->pos);
            bunch->set_pos(p->pos);
            bunch->set_type(pb_enum::PHZ_AAAAstart);
            for(auto& aaaa:p->AAAAs)
                for(auto c:aaaa.pawns())bunch->add_pawns(c);
        }
    }
    
    MeldGame::engage(game,msg);
}

bool Paohuzi::meld(Game& game,Player& player,unit_id_t card,bunch_t& bunch){
    //past,dodge and conflict
    auto past=std::find(player.unpairedCards.begin(),player.unpairedCards.end(),card)!=player.unpairedCards.end();
    if(past||player.conflictMeld){
        if(pb_enum::PHZ_ABC==bunch.type()||pb_enum::PHZ_BBB==bunch.type()){
            KEYE_LOG("meld failed, past card or conflict\n");
            return false;
        }
    }
    auto dodge=std::find(player.dodgeCards.begin(),player.dodgeCards.end(),card)!=player.dodgeCards.end();
    if(dodge){
        if(pb_enum::PHZ_BBB==bunch.type()){
            KEYE_LOG("meld failed, dodge card\n");
            return false;
        }
    }
    
    //baihuo
    if(pb_enum::PHZ_ABC==bunch.type()){
        std::vector<unit_id_t> ids;
        for(auto id:player.playData.hands())if(id/1000==card/1000 && id%100==card%100)ids.push_back(id);
        
        for(auto A:bunch.pawns()){
            for(auto it=ids.begin(),iend=ids.end();it!=iend;++it)
                if(A==*it){
                    ids.erase(it);
                    break;
                }
        }
        if(!ids.empty()){
            KEYE_LOG("meld failed, baihuo %d\n",card);
            return false;
        }
    }
    
    //erase from hands
    auto pos=player.pos;
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
    //or erase from desk,AAAs
    switch (bunch.type()) {
        case pb_enum::PHZ_BBB_B:
        case pb_enum::PHZ_AAAA:
            for(auto it=player.AAAs.begin(),iend=player.AAAs.end();it!=iend;++it){
                auto A=it->pawns(0);
                if(A/1000==card/1000 && A%100==card%100){
                    player.AAAs.erase(it);
                    break;
                }
            }
            break;
        case pb_enum::PHZ_BBBBdesk:
        case pb_enum::PHZ_AAAAdesk:
            for(auto it=player.playData.mutable_bunch()->begin(),iend=player.playData.mutable_bunch()->end();it!=iend;++it){
                auto A=it->pawns(0);
                if(A/1000==card/1000 && A%100==card%100){
                    player.playData.mutable_bunch()->erase(it);
                    break;
                }
            }
            break;
        default:
            break;
    }

    //then meld
    if(pb_enum::PHZ_ABC==bunch.type()){
        for(int i=0;i<bunch.pawns_size()/3;++i){
            auto h=player.playData.add_bunch();
            h->set_pos(bunch.pos());
            h->set_type(bunch.type());
            h->add_pawns(bunch.pawns(i*3+0));
            h->add_pawns(bunch.pawns(i*3+1));
            h->add_pawns(bunch.pawns(i*3+2));
        }
    }else{
        auto h=player.playData.add_bunch();
        h->CopyFrom(bunch);
    }
    changePos(game,pos);
    return true;
}

void Paohuzi::onMeld(Game& game,Player& player,unit_id_t card,proto3::bunch_t& bunch){
    switch (bunch.type()) {
        case proto3::OP_PASS:{
            //record past and dodge cards
            std::remove(player.unpairedCards.begin(),player.unpairedCards.end(),card);
            auto past=false;
            auto dodge=false;
            for(auto b : bunch.child()){
                if(b.type()==pb_enum::PHZ_ABC){
                    past=true;
                }else if(b.type()==pb_enum::PHZ_BBB){
                    dodge=true;
                }
            }
            if(past)player.unpairedCards.push_back(card);
            if(dodge)player.dodgeCards.push_back(card);
            break;
        }
        case proto3::PHZ_BBBBdesk:
            //record conflict meld
            if(game.token!=player.pos){
                game.players[game.token]->conflictMeld=true;
            }
            break;
        default:
            break;
    }
}

bool Paohuzi::isWin(Game& game,Player& player,unit_id_t card,std::vector<bunch_t>& output){
    auto pos=player.pos;
    auto& playdata=game.players[pos]->playData;
    auto& suite=*playdata.mutable_bunch();
    auto& hands=*player.playData.mutable_hands();
    if(hands.size()<2){
        KEYE_LOG("isWin failed: len=%d\n",hands.size());
        return false;
    }

    if(player.AAAAs.size()>=3 || player.AAAs.size()>=5){
        std::copy(player.AAAs.begin(),player.AAAs.end(),std::back_inserter(output));
        std::copy(player.AAAAs.begin(),player.AAAAs.end(),std::back_inserter(output));
        
        //add other cards for special type
        //meld [2,7,10] [1,2,3]
        std::vector<unit_id_t> all[2],two[2];
        for(auto it=hands.begin(),iend=hands.end(); it!=iend; ++it){
            auto C=*it;
            int x=(C/1000==2?1:0);
            if(C%100==1||C%100==3||C%100==7||C%100==10){
                all[x].push_back(C);
            } else if(C%100==2){
                two[x].push_back(C);
            }
        }
        for(int i=0;i<2;++i){
            //按大小分
            for(auto i2=two[i].begin();i2!=two[i].end();){
                //找一张二的组合
                std::vector<bunch_t> suites;
                hint(game,*i2,all[i],suites);
                if(suites.empty())
                    //没有，完事儿
                    i2=two[i].end();
                else{
                    //加入到算分，剔除
                    output.push_back(suites.front());
                    auto& cards=output.front().pawns();
                    for(auto ih=cards.begin(),ihh=cards.end();ih!=ihh;++ih)
                        for(auto a=all[i].begin(),aa=all[i].end();a!=aa;++a)
                            if(*ih==*a){
                                all[i].erase(a);
                                break;
                            }
                    //下一张二
                    ++i2;
                }
            }
        }
        
        // 从手牌中删除成
        for(auto s=output.begin(),ss=output.end();s!=ss;++s)
            for(auto j:s->pawns()){
                for(auto k=hands.begin(),kk=hands.end();k!=kk;++k)
                    if(j==*k){
                        hands.erase(k);
                        break;
                    }
            }
        //剩下的随便组吧
        auto sz=hands.size();
        if(sz>0){
            auto I=(sz+2)/3;
            std::vector<bunch_t> suites(I);
            for(int i=0;i<sz;++i){
                auto x=i/3;
                auto& suite=suites[x];
                suite.set_type(pb_enum::UNKNOWN);
                suite.add_pawns(hands.Get(i));
            }
            std::copy(suites.begin(),suites.end(),std::back_inserter(output));
        }
        
        return true;
    }
    
    //can't win hand card if not fire
    auto bDraw=(!validId(card)||game.pileMap.find(card)!=game.pileMap.end());
    auto fire=(pos!=game.token && !bDraw
            &&   (game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_HY||
                  game.category==pb_enum::PHZ_XX_GHZ||game.category==pb_enum::PHZ_CZ||
                  game.category==pb_enum::PHZ_HY||game.category==pb_enum::PHZ_GX));
    if(!bDraw && !fire){
        KEYE_LOG("isWin failed: not fire and not from pile\n");
        return false;
    }
    
    std::vector<unit_id_t> cards;
    std::copy(hands.begin(),hands.end(),std::back_inserter(cards));

    //insert into hand if not in
    if(validId(card)){
        auto inhand=false;
        for(auto i:cards)if(i==card){inhand=true;break;}
        if(!inhand)cards.push_back(card);
    }

    auto sorter=std::bind(&Paohuzi::comparision,this,std::placeholders::_1,std::placeholders::_2);
    std::sort(cards.begin(),cards.end(),sorter);
    
    //check desk
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
            //over=isWin(tmpHand,allSuites);
            over=isWin(game,tmpHand,tmpSuites);
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
        over=isWin(game,hand,allSuites);
    }
    if(over){
        auto M=MaxPlayer();
        int MIN_SUITES=7;
        if(M==4)MIN_SUITES=5;//衡阳，碰胡子玩法
        if(allSuites.size()+player.AAAs.size()>=MIN_SUITES){
            auto win=false;
            if(game.category==pb_enum::PHZ_PEGHZ)
                //碰胡子判胡
                win=true;
            else{
                auto point=calcPoints(game,allSuites);
                point+=calcPoints(game,player.AAAs);
                if(point>=winPoint(game,game.category))
                    win=true;
            }
            if(win){
                std::copy(allSuites.begin(),allSuites.end(),std::back_inserter(output));
                std::copy(player.AAAs.begin(),player.AAAs.end(),std::back_inserter(output));
            }
            return win;
        }
    }
    
    return false;
}

bool Paohuzi::isWin(Game& game,std::vector<unit_id_t>& cards,std::vector<bunch_t>& allSuites){
    //recursive check if is game over
    std::vector<bunch_t> outSuites;
    std::vector<bunch_t> multiSuites;
    //copy hand
    std::vector<unit_id_t> _cards(cards);
    
    //logCards(_cards,"-----isWin:");
    //先找每张牌的组合，如果没有则返回
    for(auto i=cards.begin(),ii=cards.end(); i!=ii; ++i){
        auto card=*i;
        std::vector<bunch_t> hints;
        hint(game,card,_cards,hints);
        if(hints.empty()){
            //此牌无组合
            //log("isWin no suite for card=%d:%d",card,allCards[card]%100);
            return false;
        } else if(hints.size()==1){
            //此牌唯一组合,剔除
            //log("isWin single suite for card=%d:%d", card, allCards[card]%100);
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
            if(isWin(game,_cards,outSuites)){
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
        //log("isWin cards=%d, multiSuites=%d", _cards.size(), multiSuites.size());
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
            if(isWin(game,mcards,vm)){
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

bool Paohuzi::hint3(Game& game,Player& player,unit_id_t card,bunch_t& hints){
    //all possible AAA,AAAA,BBB,BBBB like
    auto& playdata=player.playData;
    auto pos=player.pos;
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
        if(hints.type()==pb_enum::PHZ_AAAwei&&chouWei(game,player,hints))
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
        suite.set_type(pb_enum::PHZ_ABC);
        for(auto c:jiao)suite.add_pawns(c);
        suite.add_pawns(card);
        //log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.pawns(0], suite.pawns(1], suite.pawns(2]);
    }
    if(same.size()>0&&jiao.size()>0){
        //绞，append to hints: BBb
        hints.push_back(bunch_t());
        auto& suite=hints.back();
        suite.set_type(pb_enum::PHZ_ABC);
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
        size_t count = std::min( FontABackID[i].size(), FontABackID[i+1].size() );
        for ( size_t k = 0; k < count; ++k ) {
            hints.push_back(bunch_t());
            auto& suite=hints.back();
            suite.set_type(pb_enum::PHZ_ABC);
            suite.add_pawns(FontABackID[i][0]);
            suite.add_pawns(FontABackID[i+1][0]);
            suite.add_pawns(card);
            if(k==0)
                break;	//skip duplicated
        }
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
            
            auto sz=std::min(E.size(),F.size());
            for(int e=0;e<sz;++e){
                hints.push_back(bunch_t());
                auto& suite=hints.back();
                suite.set_type(pb_enum::PHZ_ABC);
                suite.add_pawns(E[0]);
                suite.add_pawns(F[0]);
                suite.add_pawns(card);
                //log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.pawns(0], suite.pawns(1], suite.pawns(2]);
                if(e==0)break;	//skip duplicated
            }
        }
    }
}

pb_enum Paohuzi::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_INVALID;
    auto type=fixOps(bunch.type());
    auto sz=bunch.pawns_size();
    switch(type) {
        case pb_enum::PHZ_AAA:
        case pb_enum::PHZ_AAAwei:
        case pb_enum::PHZ_AAAchou:
        case pb_enum::PHZ_BBB:
            if(sz==3){
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
            if(sz==4){
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
            if(sz%3==0){
                auto it=bunch.pawns().begin();
                auto ok=true;
                for(auto i=0;i<sz/3;++i,it+=3){
                    std::vector<unit_id_t> cards(it,it+3);
                    auto sorter=std::bind(&Paohuzi::comparision,this,std::placeholders::_1,std::placeholders::_2);
                    std::sort(cards.begin(),cards.end(),sorter);
                    
                    auto A=cards[0];
                    auto B=cards[1];
                    auto C=cards[2];
                    ok=(
                        (A/1000==B/1000 && A/1000==C/1000 &&
                         ((A%100+1==B%100 && A%100+2==C%100) || (A%100==2 && B%100==7 && C%100==10))
                         ) ||
                        
                        (A%100==B%100 && A%100==C%100 && !(A/1000==B/1000 && A/1000==C/1000))
                        );
                    if(!ok)break;
                }
                if(ok)bt=bunch.type();
            }
            break;
        case pb_enum::PHZ_AA:
            if(sz==2){
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

void Paohuzi::settle(Player& player,std::vector<proto3::bunch_t>& allSuites,unit_id_t card){
    //只结算不判断
    auto pos=player.pos;
    auto& game=*player.game;
    auto M=MaxPlayer();
    std::stringstream ss;//广西跑胡子用
    int _cardNum=0;//广西跑胡子用
    //清洗缓冲区
    ss.clear();
    game.spSettle=std::make_shared<MsgNCSettle>();
    auto& msg=*game.spSettle;
    
    for(pos_t i=0;i<M;++i){
        auto play=msg.add_play();
        play->set_win(pos==i&&allSuites.size()>0?1:0);
    }

    if(!game.spFinish){
        //prepare final end message
        game.spFinish=std::make_shared<MsgNCFinish>();
        for(pos_t i=0; i < M; ++i){
            game.spFinish->add_play();
        }
    }
    auto spFinalEnd=game.spFinish;
    auto spGameEnd=game.spSettle;
    auto& play=*msg.mutable_play(pos);
    auto& playFinish=*spFinalEnd->mutable_play(pos);
    
    //copy bunches
    play.clear_bunch();
    for(auto& bunch:allSuites)play.add_bunch()->CopyFrom(bunch);
    
    int point=0,score=0,multiple=0,chunk=0;
    bool self=(pos==game.token&&game.pileMap.find(card)!=game.pileMap.end());
    //如果天胡，card就被设置为-1(255)了，这里有漏洞则处理
    bool bCardError=(card>80||card<0);//说明牌非法，不能用非法的牌做放炮判断
    bool fire=(pos!=game.token&&!bCardError&&game.pileMap.find(card)!=game.pileMap.end()
               &&(game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_HY||
                  game.category==pb_enum::PHZ_XX_GHZ||game.category==pb_enum::PHZ_CZ||
                  game.category==pb_enum::PHZ_HY||game.category==pb_enum::PHZ_GX));
    
    //适配碰胡子算分规则
    switch(game.category){
        case pb_enum::PHZ_PEGHZ:
            point=-1;//置此标示，以便碰胡子和其他玩法作区分后碰胡子能够单独计算名堂并算分
            break;
        default:
            //胡息
            point=calcPoints(game,allSuites);
            break;
    }
    
    //log("settle pos=%d, suites=%d, point=%d",pos,allSuites.size(),point);
    
    std::vector<achv_t> achvs;
    if(point>=winPoint(game,game.category)){
        //胡了
        
        //先计名堂
        calcAchievement(game,game.category,allSuites,achvs);
        
        //天胡
        auto naturalWin=false;
        if(naturalWin&&game.category!=pb_enum::PHZ_SY
           &&game.category!=pb_enum::PHZ_CD_HHD&&game.category!=pb_enum::PHZ_HY){
            //天胡
            achvs.push_back(achv_t());
            auto& ach=achvs.back();
            ach.set_type(pb_enum::WIN_TIAN);
            if(game.category==pb_enum::PHZ_SYBP)
                ach.set_key(pb_enum::ACHV_KEY_SCORE);
            else if(game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ)
                ach.set_key(pb_enum::ACHV_KEY_POINT);
            else if(game.category==pb_enum::PHZ_PEGHZ)
                ach.set_key(pb_enum::ACHV_KEY_SCORE);
            else
                ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][game.category]);
        }
        
        //地胡和放炮可能产生冲突，要先判断地胡后再对放炮做处理
        //地胡，听胡
        if(game.pile.size()>=19&&game.category!=pb_enum::PHZ_SY&&
           game.category!=pb_enum::PHZ_CD_HHD&&game.category!=pb_enum::PHZ_HY){
            //地胡：1，闲家；2，闲家无进张；3，牌堆满的
            int drawCount=0;
            for(auto& p:game.players)drawCount+=p->inputCount;
            if(game.firstCard==card && game.banker!=pos&&drawCount==0){
                //地胡时，有时候会检测到放炮的名堂，但是是不允许的，这里在地胡处理中直接屏蔽
                fire=false;
                
                achvs.push_back(achv_t());
                auto& ach=achvs.back();
                ach.set_type(pb_enum::WIN_DI);
                auto nn=nnn[ach.type()][game.category];
                if(game.category==pb_enum::PHZ_SYBP)
                    ach.set_key(pb_enum::ACHV_KEY_SCORE);
                else if(game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ)
                    ach.set_key(pb_enum::ACHV_KEY_POINT);
                else
                    ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                ach.set_value(nn);
            }
        }
        
        //处理湘乡告胡子点炮时胡息加10的情况，以便后续翻番计算
        int firePoint=0;
        if(fire&&game.category==pb_enum::PHZ_XX_GHZ){
            firePoint+=10;
        }
        //自摸
        if(self){
            if(game.category!=pb_enum::PHZ_SY&&game.category!=pb_enum::PHZ_CD_HHD&&
               game.category!=pb_enum::PHZ_CS){
                achvs.push_back(achv_t());
                auto& ach=achvs.back();
                ach.set_type(pb_enum::WIN_SELF);
                auto nn=nnn[ach.type()][game.category];
                if(game.category==pb_enum::PHZ_SYBP)
                    ach.set_key(pb_enum::ACHV_KEY_SCORE);
                else if(game.category==pb_enum::PHZ_CD_QMT||game.category==pb_enum::PHZ_HH)
                    ach.set_key(pb_enum::ACHV_KEY_CHUNK);
                else if(game.category==pb_enum::PHZ_XX_GHZ)
                    ach.set_key(pb_enum::ACHV_KEY_POINT);
                else
                    ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                ach.set_value(nn);
            }
        }
        
        if(game.category==pb_enum::PHZ_CD_QMT){
            //听胡
            if(player.inputCount<=0){
                achvs.push_back(achv_t());
                auto& ach=achvs.back();
                ach.set_type(pb_enum::WIN_TING);
                ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                ach.set_value(nnn[ach.type()][game.category]);
            }
            //单调将
            if(player.lastHand)
                for(auto i=allSuites.begin(),ii=allSuites.end();i!=ii;++i){
                    if(i->type()==pb_enum::PHZ_AA){
                        for(auto j:i->pawns()){
                            if(j==card){
                                achvs.push_back(achv_t());
                                auto& ach=achvs.back();
                                ach.set_type(pb_enum::WIN_MONKEY);
                                ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                                ach.set_value(nnn[ach.type()][game.category]);
                                break;
                            }
                        }
                    }
                }
        }
        //荒番
        if(game.noWinner>0&&game.category==pb_enum::PHZ_CD_QMT){
            achvs.push_back(achv_t());
            auto& ach=achvs.back();
            ach.set_type(pb_enum::WIN_YELLOW);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][game.category]+game.noWinner-1);
            //赢牌后重计荒庄次数
            game.noWinner=0;
        }
        
        
        if(game.category==pb_enum::PHZ_LD){
            //卡胡
            if(point==30){
                achvs.push_back(achv_t());
                auto& ach=achvs.back();
                ach.set_type(pb_enum::WIN_30);
                ach.set_key(pb_enum::ACHV_KEY_POINT);
                ach.set_value(nnn[ach.type()][game.category]);
                point=nnn[ach.type()][game.category];
            } else if(point==20){
                achvs.push_back(achv_t());
                auto& ach=achvs.back();
                ach.set_type(pb_enum::WIN_20);
                ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                ach.set_value(nnn[ach.type()][game.category]);
            }
        }
        
        //根据名堂算分算胡之前，先对七方门子是否达到30胡息做计算
        //在湘乡告胡子中，标示30胡和红胡是否都有的情况：bN30Red-false存在，true不存在;bN13Red标示红乌
        int pt=0;
        if(game.category==pb_enum::PHZ_XX_GHZ){
            //湘乡30胡和30胡加10红
            bool bRed_=false,bBlack=false,b13Red=false;//标示是否有红胡的名堂
            std::vector<achv_t>::iterator iRed=achvs.end();
            for(auto ia=achvs.begin(),iaa=achvs.end();ia!=iaa;++ia){
                //检查是否有红胡，红乌，黑胡，三者互斥
                if(ia->type()==pb_enum::WIN_RED){
                    iRed=ia;
                    bRed_=true;
                    break;
                } else if(ia->type()==pb_enum::WIN_BLACK){
                    bBlack=true;
                    break;
                } else if(ia->type()==pb_enum::WIN_13RED){
                    iRed=ia;
                    b13Red=true;
                }
                
            }
            if(point>=30){
                //30胡
                if(bRed_||b13Red){
                    //这里需要把红胡或者红乌的名堂去掉
                    if(iRed!=achvs.end()){
                        achvs.erase(iRed);
                    }
                    pt+=100;
                    achvs.push_back(achv_t());
                    auto& ach=achvs.back();
                    ach.set_type(pb_enum::WIN_10RED);
                } else{
                    //只是达到30息及以上了，算两番
                    achvs.push_back(achv_t());
                    auto& ach=achvs.back();
                    ach.set_type(pb_enum::WIN_30);
                    ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
                    ach.set_value(2);
                }
            }
        }//pb_enum::PHZ_XX_GHZ
        
        //胡息
        for(auto a=achvs.begin(),aa=achvs.end();a!=aa;++a){
            if(a->type()==pb_enum::ACHV_KEY_POINT)
                pt+=a->value();
        }
        
        if((game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ)&&pt>=100)
            //pt>=100，因为对于湘乡告胡子，自摸是加10胡息，其他的明天都是>=100胡息的
            point=pt;
        else
            point+=pt;
        //这个加法是针对湘乡告胡子放炮加分的，保持流程统一，故放在此处
        point+=firePoint;
        
        
        //放炮最高100息
        if(fire&&(game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ)&&point>100)point=100;
        //囤数
        chunk+=calcScore(game,game.category,point);
        for(auto a=achvs.begin(),aa=achvs.end();a!=aa;++a){
            if(a->type()==pb_enum::ACHV_KEY_CHUNK)
                chunk+=a->value();
        }
        //番数
        for(auto a=achvs.begin(),aa=achvs.end();a!=aa;++a)
            if(a->type()==pb_enum::ACHV_KEY_MULTIPLE)
                multiple+=a->value();
        
        if(game.category!=pb_enum::PHZ_LD){
            if(multiple==0)multiple=multiple=1;
        }
        else if(game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ){
            multiple=(int)std::pow(2,multiple);
        }
        //番数封顶判断
        if(game.category==pb_enum::PHZ_HH||game.category==pb_enum::PHZ_CS){
            //处理
//            multiple=calcMultiOrScore(game,game.category,multiple);
        }
        
        //分数
        if(game.category!=pb_enum::PHZ_LD||game.category!=pb_enum::PHZ_XX_GHZ)
            score=chunk*multiple;
        else
            //score=chunk*pow(2,multiple);
            score=chunk*multiple;
        for(auto a=achvs.begin(),aa=achvs.end();a!=aa;++a)
            if(a->type()==pb_enum::ACHV_KEY_SCORE)
                score+=a->value();
        //log("end point=%d, chunk=%d, multiple=%d, score=%d",point,chunk,multiple,score);
        
        //换庄
        game.bankerChanged=(game.banker!=pos);
        game.banker=pos;
        //点炮
        if(fire){
            achvs.push_back(achv_t());
            auto& ach=achvs.back();
            ach.set_type(pb_enum::WIN_FIRE);
            ach.set_key(pb_enum::ACHV_KEY_SCORE);
            ach.set_value(score);
        }
        
        //所有名堂和分数计算完毕后处理广西跑胡子番醒
        //对于广西跑胡子，因为有番醒，因此要在给客户端发送桌牌之前处理番醒
        if(game.category==pb_enum::PHZ_GX){
            //计算番醒
//            calcCardScore(game,game.category,pos,ss,_cardNum);
//            msg.m_xingStr=ss.str();//将番醒组合给客户端，以便展示使用
            //算番结束将醒分加到基础分上
            score+=_cardNum;//番醒只是胡牌者的分数
        }
        
        //scores存当局分数变动，totalscores存累计分数
        if(game.category==pb_enum::PHZ_SYBP||game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_XX_GHZ){
            //差额玩法
            play.set_score(score);
            playFinish.set_score(playFinish.score());
//            msg.m_scores[pos]=score;
//            spFinalEnd->m_total[pos].score+=score;
            
            if(fire){
                //放炮，两人变动
                msg.mutable_play(game.token)->set_score(-score);
                game.spFinish->mutable_play(game.token)->set_score(game.spFinish->mutable_play(game.token)->score()-score);
//                msg.m_scores[game.token]=-score;	//放炮这要扣分的，有番数
//                spFinalEnd->m_total[game.token].score-=score;
            }
//            for(int i=0; i<M; ++i)
//                msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
        } else{
            //两倍分数哦
            if(fire){
                //放炮
                //适配郴州特殊算分规则和4人玩法
                bool bMulti3=false;
                switch(game.category){
                    case pb_enum::PHZ_CZ:
                    case pb_enum::PHZ_HY:
                        bMulti3=true;	break;
                    default:
                        break;
                }
                play.set_score(bMulti3?score*3:score*2);
                msg.mutable_play(game.token)->set_score(-(bMulti3?score*3:score*2));
//                msg.m_scores[pos]=bMulti3?score*3:score*2;
//                msg.m_scores[game.token]=-(bMulti3?score*3:score*2);
            } else{
                //适配4人玩法
                int multi=M-1;
                if(game.category==pb_enum::PHZ_CD_HHD||game.category==pb_enum::PHZ_CD_QMT)
                {
//                    score=calcMultiOrScore(game.category,score*multi);
                    for(int i=0; i<M; ++i){
                        msg.mutable_play(i)->set_score(i==pos?score:-(score/multi));
//                        msg.m_scores[i]=(i==pos?score:-(score/multi));
                    }
                } else{
                    for(int i=0; i<M; ++i)
                        msg.mutable_play(i)->set_score(i==pos?score*multi:-score);
//                        msg.m_scores[i]=(i==pos?score*multi:-score);
                }
            }
            for(int i=0; i<M; ++i){
                game.spFinish->mutable_play(game.token)->set_score(game.spFinish->mutable_play(game.token)->score()+msg.play(i).score());
//                spFinalEnd->m_total[i].score+=msg.m_scores[i];
//                msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
            }
        }
    } else if(point>=0){
        if(game.category==pb_enum::PHZ_CZ){
            //郴州毛胡子玩法，荒庄需要换庄
            auto M=MaxPlayer();
            auto p=(game.banker+1)%M;
            game.bankerChanged=true;
            game.banker=p;
            changePos(game,p);
        }
        
        //荒庄
        ++game.noWinner;
        game.bankerChanged=false;
        if(game.category==pb_enum::PHZ_SYBP||game.category==pb_enum::PHZ_XX_GHZ)
            //剥皮和湘乡告胡子，庄家扣10分
            msg.mutable_play(game.banker)->set_score(-10);
        for(int i=0; i<M; ++i){
            game.spFinish->mutable_play(game.token)->set_score(game.spFinish->mutable_play(game.token)->score()+msg.play(i).score());
//            spFinalEnd->m_total[i].score+=msg.m_scores[i];
//            msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
        }
    } else{
        //pb_enum::PHZ_PENGHUZI
        //碰胡子胡牌类型检查
        if(game.category==pb_enum::PHZ_PEGHZ){
            //记录碰牌胡和偎牌胡时候的三张组合牌的数量
            auto& h=game.players[pos]->playData.hands();
            std::vector<unit_id_t> hand(h.begin(),h.end());
            auto kNum=findSuiteKT(game,hand,0,pos);
            
            auto ops=pb_enum::BUNCH_INVALID;
            for(auto& bun:allSuites){
                for(auto c:bun.pawns())if(c==card){
                    ops=bun.type();
                    break;
                }
                if(ops!=pb_enum::BUNCH_INVALID)
                    break;
            }
            auto resSuite=fixOps(ops);
            //if-else 结构处理，因为三大，四大，偎碰胡之间存在优先级关系
            if(resSuite==pb_enum::PHZ_AAAwei||resSuite==pb_enum::PHZ_AAAchou){
                if(kNum==4){
                    //四大 胡
                    game.players[pos]->m_winMark=pb_enum::WINSID;
                } else if(kNum==3){
                    //三大 胡
                    game.players[pos]->m_winMark=pb_enum::WINSAND;//3标示三大胡
                }else{
                    //偎胡
                    game.players[pos]->m_winMark=pb_enum::WINWEI;
                }
            } else if(resSuite==pb_enum::PHZ_BBB){
                if(kNum==4){
                    //四大 胡
                    game.players[pos]->m_winMark=pb_enum::WINSID;
                } else if(kNum==3){
                    //三大 胡
                    game.players[pos]->m_winMark=pb_enum::WINSAND;//3标示三大胡
                } else{
                    //碰胡
                    game.players[pos]->m_winMark=pb_enum::WINPENG;
                }
            }else if(resSuite==pb_enum::PHZ_BBB_B||resSuite==pb_enum::PHZ_BBBBdesk){
                //跑牌胡
                bool bWei=true;//checkWeiPengAct(suite,pos,card);//true:偎坎牌跑胡，false:碰牌跑胡
                if(resSuite==pb_enum::PHZ_BBB_B){
                    game.players[pos]->m_winMark=pb_enum::WINWPAO;//偎坎跑胡
                } else if(resSuite==pb_enum::PHZ_BBBBdesk){
                    if(bWei){
                        //偎坎跑胡
                        game.players[pos]->m_winMark=pb_enum::WINWPAO;//偎坎跑胡
                    } else{
                        //碰跑胡
                        game.players[pos]->m_winMark=pb_enum::WINPPAO;//碰跑胡
                    }
                }
            }else if(resSuite==pb_enum::PHZ_AAAA||resSuite==pb_enum::PHZ_AAAAdesk||resSuite==pb_enum::PHZ_AAAAstart){
                //提牌胡
                game.players[pos]->m_winMark=pb_enum::WINTI;
            } else{
                //平胡
                game.players[pos]->m_winMark=pb_enum::WINPING;
            }
            
            //记录胡牌次数
            //_desk->m_winCount[pos]++;
        }
        
        //地胡不能有进张的判断条件
        auto markInput=0;
        //放炮否
        bool pfire=(pos!=game.token&&!bCardError&&game.pileMap.find(card)==game.pileMap.end()
                    &&(game.category==pb_enum::PHZ_PEGHZ));
        //地胡，听胡
        for(auto i=0;i<M;++i){
            if(i==game.banker)continue;
            if(game.players[i]->inputCount==0) markInput++;
        }
        
        //计算胡牌算分
        if(game.m_winPeo>0&&pos!=-1){
            //天胡，双龙，小七对也进来处理
            if(game.pile.size()>=23){
                //地胡：1，闲家；2，闲家无进张；3，牌堆满的；4，庄家出了一张牌
                if(game.banker!=pos&&markInput==3&&game.players[game.banker]->playData.hands_size()==14){
                    //地胡时，有时候会检测到放炮的名堂，但是是不允许的，这里在地胡处理中直接屏蔽
                    pfire=false;
                    achvs.push_back(achv_t());
                    auto& ach=achvs.back();
                    ach.set_type(pb_enum::WIN_DI);
                    ach.set_key(pb_enum::ACHV_KEY_SCORE);
                    ach.set_value(nnn[ach.type()][game.category]);
                }
            }
            //赢了，记录连胡次数,
            game.players[pos]->winCount++;
            //首先计算名堂
            calcPengAchievement(game,game.category,allSuites,achvs,pos);
            
            auto winScore=0;
            int lastScore=4;//胡牌基础分数4分
            bool bTian=false,b5FU=false;//标示天胡，双龙，小七对，如果是这几种情况，则直接结算
            bool b7DualLog=false;//标示有双龙或者小七对时，不计算起手提坎分数
            
            
            //胡牌算分结束，计算名堂分数
            for(auto iv=achvs.begin(),ivv=achvs.end();iv!=ivv;++iv){
                if(iv->type()==pb_enum::WIN_TIAN){
                    //天胡
                    bTian=true;
                    lastScore=0;//不计算初始的4分
                    lastScore=iv->value();
                } else if(iv->type()==pb_enum::WIN_7PAIR||iv->type()==pb_enum::WIN_DUALDRA){
                    //小七对,双龙
                    bTian=true;
                    lastScore=0;//不计算初始的4分
                    b7DualLog=true;
                    lastScore=iv->value();
                } else if(iv->type()==pb_enum::WIN_DI){
                    //地胡
                    if(pfire)
                        pfire=false;//防止重复计算放炮名堂
                    lastScore=0;//不计算初始的4分
                    lastScore=iv->value();
                } else if(iv->type()==pb_enum::WIN_5FU){
                    //五福
                    b5FU=true;
                    lastScore=iv->value();
                }
            }
            if(!bTian&&!b5FU){
                //处天胡，双龙，小七对之外的胡牌算分处理，否则直接结算
                //放炮
                if(pfire){
                    achvs.push_back(achv_t());
                    auto& ach=achvs.back();
                    ach.set_type(pb_enum::WIN_FIRE);
                    ach.set_key(pb_enum::ACHV_KEY_SCORE);
                    ach.set_value(nnn[ach.type()][game.category]);
                    //放炮有偎，提，碰，跑，等情况，这里分别计分
                    switch(game.players[pos]->m_winMark){
                        case pb_enum::WINSAND:
                            //到这里，能说明就是碰的胡的三大胡牌方式
                            lastScore+=5;
                            break;
                        case pb_enum::WINSID:
                            //到这里，能说明就是碰的四大胡牌方式
                            lastScore+=9;
                            break;
                        case pb_enum::WINPENG:
                            lastScore+=1;//最后在放炮出，处理结算
                            break;
                        case pb_enum::WINPPAO:
                            //碰跑胡
                            lastScore+=4;
                            break;
                        default:
                            //平胡
                            break;
                    }
                } else{
                    //堆上胡牌
                    switch(game.players[pos]->m_winMark){
                        case pb_enum::WINSAND:
                            //到这里，能说明就是偎的三大胡牌方式
                            lastScore+=6;
                            break;
                        case pb_enum::WINSID:
                            //到这里，能说明就是偎的四大胡牌方式
                            lastScore+=10;
                            break;
                        case pb_enum::WINWEI:
                            lastScore+=2;
                            break;
                        case pb_enum::WINTI:
                            lastScore+=8;
                            break;
                        case pb_enum::WINPENG:
                            lastScore+=1;//最后在放炮出，处理结算
                            break;
                        case pb_enum::WINWPAO:
                            //偎坎跑胡
                            lastScore+=4;
                            break;
                        case pb_enum::WINPPAO:
                            //碰跑胡
                            lastScore+=4;
                            break;
                        default:
                            //平胡
                            break;
                    }
                }
            }
            
            //双龙不计起手提坎分数，这里处理掉
            if(b7DualLog){
                lastScore=40;
                for(auto i=0;i<M;++i)
                    game.players[i]->playData.set_score(0);
                    //game.m_score[i]=0;
            }
            
            //胡牌番数含义：中庄x2:0 中庄翻番：1 连中：2   注：有5福名堂
            if(pos==game.banker&&game.players[pos]->winCount>1&&!b5FU){
                //只有庄家赢了才做，中庄X2，连胡，中庄翻番的限制计算
                if(game._multiScore==0){
                    //中庄算胡分
                    //连胡处理
                    lastScore*=2;
                } else if(game._multiScore==1){
                    //中庄翻番算胡分
                    lastScore*=std::pow(2,game.players[pos]->winCount-1);
                } else if(game._multiScore==2){
                    //连中算胡分
                    lastScore+=4;
                }
            }
            
            winScore+=lastScore;
            //结算
            if(pfire){
                //放炮的结算
                game.players[pos]->playData.set_score(game.players[pos]->playData.score()+winScore*(M-1));
                game.players[game.token]->playData.set_score(game.players[game.token]->playData.score()-winScore*(M-1));
            } else{
                game.players[pos]->playData.set_score(game.players[pos]->playData.score()+winScore*(M-1));
                for(auto i=0;i<M;++i){
                    if(pos==i)continue;
                    game.players[i]->playData.set_score(game.players[i]->playData.score()-winScore);
                }
            }
            
            //最后将没赢的其他玩家的连胡记录清零
            for(auto i=0;i<M;++i){
                if(pos==i)continue;
                game.players[i]->winCount=0;
            }
            
            
            //封装消息
            
            //换庄
            game.bankerChanged=(game.banker!=pos);
            if(game.bankerChanged){
                //说明庄家输了 ，连胡计数要归零
                game.players[game.banker]->winCount=0;
            }
            game.banker=pos;
            
            for(int i=0; i<M; ++i){
                game.spFinish->mutable_play(i)->set_score(game.spFinish->mutable_play(i)->score()+msg.play(i).score());
                //spFinalEnd->m_total[i].score+=msg.m_scores[i];
                //msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
            }
        } else{
            //荒庄后者输了，连胡记录都要清零从新计数
            //到这里，pos可能非法，
            for(auto i=0;i<M;++i)
                game.players[i]->winCount=0;
            game.bankerChanged=false;
            for(int i=0; i<M; ++i){
                game.spFinish->mutable_play(i)->set_score(game.spFinish->mutable_play(i)->score()+msg.play(i).score());
                //spFinalEnd->m_total[i].score+=msg.m_scores[i];
                //msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
            }
        }
    }//pb_enum::PHZ_PENGHUZI
    
    for(auto c:game.pile)spGameEnd->mutable_pile()->Add(c);
    play.set_point(point);
    play.set_chunk(chunk);
    play.set_multiple(multiple);
/*
    if(game.category==pb_enum::PHZ_SYBP){	//专门为客户端显示做的
        int s=(game.noWinner>0?-10:0);
        for(auto a=achvs.begin(),aa=achvs.end();a!=aa;++a)
            s+=a->score;
        msg.m_multiples=s;
    }
*/
    //achvs
    for(auto& achv:achvs)play.add_achvs()->CopyFrom(achv);
    //bunches and hands
    for(int i=0; i<M; ++i){
        //logHands(i);
        auto& destPlay=*msg.mutable_play(i);
        auto& srcPlay=game.players[i]->playData;
        for(auto& src:srcPlay.bunch())
            destPlay.add_bunch()->CopyFrom(src);
        
        for(auto& src:srcPlay.hands())
            destPlay.add_hands(src);
    }
    
    //the final end message
    if (pos!=-1) {
        //fill final end messasge
        playFinish.set_win(playFinish.win());
        playFinish.set_point(playFinish.point()+point);
        playFinish.set_chunk(playFinish.chunk()+chunk);
        playFinish.set_multiple(playFinish.multiple()+multiple);
    }
}

void Paohuzi::calcAchievement(Game& game,pb_enum rule,const std::vector<bunch_t>& suites,std::vector<achv_t>& avs){
    //统计工作
    int red=0,big=0,small=0;
    auto pair=true;
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
        int ops=i->type();	ops=fixOps((pb_enum)ops);
        if(pair&&(ops==pb_enum::PHZ_ABC||ops==pb_enum::UNKNOWN))pair=false;
    }
    
    //海胡
    if(last){
        if(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_GX){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_LAST);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    }
    
    //红乌
    if(red>=10&&(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_XX_GHZ)){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        if(red>=13){
            ach.set_type(pb_enum::WIN_13RED);
            ach.set_key(pb_enum::ACHV_KEY_POINT);
            ach.set_value(nnn[ach.type()][rule]);
        } else{
            ach.set_type(pb_enum::WIN_RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    }
    if(red>=10&&(rule==pb_enum::PHZ_SY)){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        ach.set_type(pb_enum::WIN_RED);
        ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
        ach.set_value(nnn[ach.type()][rule]);
    }
    //红胡
    if(red>=10&&(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT)){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        ach.set_type(pb_enum::WIN_RED);
        ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
        ach.set_value(nnn[ach.type()][rule]+red-10);
    }
    if(rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_HHD){
        if(red>=13&&rule!=pb_enum::PHZ_HH){
            //红乌
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_13RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }else if(red>=10){
            //红胡
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            auto v=nnn[ach.type()][rule];
            if(rule==pb_enum::PHZ_HH)
                v+=(red-10);
            ach.set_value(v);
        }
    }
    if(red==2){
        //双飘
        if(rule==pb_enum::PHZ_CS){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_DOUBLE);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    } else if(red==1){
        //点胡
        if(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT||
           rule==pb_enum::PHZ_CD_HHD||rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_SINGLE);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    } else if(red==0){
        //黑胡
        if(rule==pb_enum::PHZ_SY||rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_HH||
           rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_CD_HHD||
           rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_XX_GHZ){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_BLACK);
            if(rule==pb_enum::PHZ_LD||rule==pb_enum::PHZ_XX_GHZ)
                ach.set_key(pb_enum::ACHV_KEY_POINT);
            else
                ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    }
    //二三四比
    if(rule==pb_enum::PHZ_CS){
        if(red==2&&((redmap[2]==2&&redmap[7]==0&&redmap[10]==0)||
                    (redmap[2]==0&&redmap[7]==2&&redmap[10]==0)||
                    (redmap[2]==0&&redmap[7]==0&&redmap[10]==2))){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_2RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }else if(red==3&&((redmap[2]==3&&redmap[7]==0&&redmap[10]==0)||
                          (redmap[2]==0&&redmap[7]==3&&redmap[10]==0)||
                          (redmap[2]==0&&redmap[7]==0&&redmap[10]==3))){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_3RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }else if(red==4&&((redmap[2]==4&&redmap[7]==0&&redmap[10]==0)||
                          (redmap[2]==0&&redmap[7]==4&&redmap[10]==0)||
                          (redmap[2]==0&&redmap[7]==0&&redmap[10]==4))){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_4RED);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    }
    //一块匾
    int _count=0;
    for(auto ic=suites.begin(),icc=suites.end();ic!=icc;++ic){
        //考虑到可能最后是吃，碰，跑，提偎等赢的，应该首先把应的状态去掉在做下边的处理即可
        auto res=fixOps(ic->type());
        if(res!=pb_enum::PHZ_ABC&&res!=pb_enum::PHZ_AAA&&res!=pb_enum::PHZ_AAAwei&&res!=pb_enum::PHZ_AAAchou&&res!=pb_enum::PHZ_BBB&&res!=pb_enum::PHZ_AAAA&&res!=pb_enum::PHZ_AAAAstart&&res!=pb_enum::PHZ_BBB_B&&res!=pb_enum::PHZ_AAAAdesk)
            continue;
        int count=0;
        for(auto A:ic->pawns()){
            if(A%100==2||A%100==7||A%100==10)
                count++;
        }
        if((count==3||count==4)&&(red==3||red==4)){
            _count++;
        }
    }
    if(_count==1&&rule==pb_enum::PHZ_LD){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        ach.set_type(pb_enum::WIN_PLATE);
        ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
        ach.set_value(nnn[ach.type()][rule]);
    }

    //大小胡
    if(rule==pb_enum::PHZ_CS||rule==pb_enum::PHZ_CD_QMT||rule==pb_enum::PHZ_HH){
        if(big>=18){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_BIG);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]+big-18);
        }
        int S=(rule==pb_enum::PHZ_CS?18:16);
        if(small>=S){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_SMALL);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]+small-S);
        }
    }
    //对胡
    if(pair){
        if(rule==pb_enum::PHZ_CS||
           rule==pb_enum::PHZ_HH||rule==pb_enum::PHZ_CD_QMT){
            avs.push_back(achv_t());
            auto& ach=avs.back();
            ach.set_type(pb_enum::WIN_PAIR);
            ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
            ach.set_value(nnn[ach.type()][rule]);
        }
    }
}

//计算碰胡子的名堂
void Paohuzi::calcPengAchievement(Game& game,pb_enum rule,const std::vector<bunch_t>& suites,std::vector<achv_t>& avs,int pos){
    //TODO:碰胡子名堂计算
    auto& h=game.players[pos]->playData.hands();
    std::vector<unit_id_t> hand(h.begin(),h.end());
    auto kNum=findSuiteKT(game,hand,0,pos);
    bool bPaoDaul=false;//跑双标示
    for(auto iv=suites.begin(),ivv=suites.end();iv!=ivv;++iv){
        if(iv->type()==pb_enum::PHZ_AAAA||iv->type()==pb_enum::PHZ_AAAAdesk||iv->type()==pb_enum::PHZ_BBB_B||iv->type()==pb_enum::PHZ_BBBBdesk){
            //判断是否跑双
            bPaoDaul=true;
        }
    }
    //五福
    if(kNum==5/*&&_desk->m_alarm[pos]*/){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        ach.set_type(pb_enum::WIN_5FU);
        ach.set_key(pb_enum::ACHV_KEY_SCORE);
        ach.set_value(nnn[ach.type()][rule]);
    }
    
    //跑双
    //if(bPaoDaul){
    //avs.push_back(AchievementData());
    //auto& ach=avs.back();
    //ach.type=pb_enum::WIN_RUNDUAL;
    //ach.score=nnn[ach.type][rule];
    //}
    
    //连胡名堂
    if(game.players[pos]->winCount>1){
        avs.push_back(achv_t());
        auto& ach=avs.back();
        //乘方算，这里静态数组中存放乘方的底数
        ach.set_type(pb_enum::WIN_REPEAT);
        ach.set_key(pb_enum::ACHV_KEY_MULTIPLE);
        ach.set_value(nnn[ach.type()][rule]);
    }
}

int Paohuzi::findSuiteKT(Game& game,std::vector<unit_id_t> hands,int type,int pos){
    auto kanNum=0,tpNum=0,res=0;
    std::vector<std::vector<unit_id_t>> mc[2];
    mc[0].resize(10);
    mc[1].resize(10);
    for(auto it=hands.begin(),iend=hands.end(); it!=iend; ++it){
        auto C=*it;
        int x=(C/1000==1?1:0);
        auto& v=mc[x][C%100-1];
        v.push_back(C);
    }
    for(auto i=0;i<2;++i){
        //分别在大小牌中寻找坎牌
        auto& v=mc[i];
        for(auto iv=v.begin(),ivv=v.end();iv!=ivv;++iv){
            if(iv->size()==3){
                kanNum++;
            }
        }
    }
    if(pos>=0){
        auto& suite=game.players[pos]->playData.bunch();
        for(auto it=suite.begin(),itt=suite.end();it!=itt;++it){
            auto resOps=fixOps(it->type());
            if(it->pawns_size()==3&&(resOps==pb_enum::PHZ_BBB||resOps==pb_enum::PHZ_AAA||resOps==pb_enum::PHZ_AAAchou||resOps==pb_enum::PHZ_AAAwei)){
                kanNum++;
            } else if(it->pawns_size()==4&&(resOps==pb_enum::PHZ_AAAA||resOps==pb_enum::PHZ_AAAAstart||resOps==pb_enum::PHZ_AAAAdesk||resOps==pb_enum::PHZ_BBBBdesk||resOps==pb_enum::PHZ_BBB_B)){
                tpNum++;
            }
        }
    }
    if(type==0){
        //标示查看碰，坎，偎组合数量
        res=kanNum;
    }
    if(type==1){
        //标示查看提，跑组合数量
        res=tpNum;
    }
    return res;
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

bool Paohuzi::chouWei(Game& game,Player& player,bunch_t& bunch){
    //臭偎
    auto& vp=player.unpairedCards;
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

bool Paohuzi::checkDiscard(Player& player,unit_id_t drawCard){
    auto sz=player.playData.hands_size();
    if(sz<=0)
        return false;
    
    auto aaaa=(!player.AAAAs.empty());
    if(!aaaa){
        auto& bunches=player.playData.bunch();
        for(auto& bunch:bunches){
            if(bunch.pawns_size()>3){
                aaaa=true;
                break;
            }
        }
    }

    if(drawCard!=invalid_card)sz++;
    auto ret=(sz%3==(aaaa?2:0));
    if(ret){
        MeldGame::checkDiscard(player,drawCard);
        player.game->pendingDiscard->bunch.add_pawns(drawCard);
    }
    
    return ret;
}

int opWeight(pb_enum op){
    int i=0;
    auto ops=op;
    op=fixOps(op);
    switch(op){
        case pb_enum::PHZ_ABC:
            i=1;break;
        case pb_enum::PHZ_BBB:
        case pb_enum::PHZ_AAA:
        case pb_enum::PHZ_AAAwei:
        case pb_enum::PHZ_AAAchou:
            i=2;break;
        case pb_enum::PHZ_BBB_B:
        case pb_enum::PHZ_BBBBdesk:
        case pb_enum::PHZ_AAAAstart:
        case pb_enum::PHZ_AAAA:
        case pb_enum::PHZ_AAAAdesk:
            i=3;break;
        case pb_enum::UNKNOWN:
        case pb_enum::PHZ_AA:
        //case pb_enum::OP_PASS:
        //case pb_enum::PLAY:
        default:
            i=0;break;
    }
    return (ops>pb_enum::BUNCH_WIN?i+(ops/pb_enum::BUNCH_WIN*pb_enum::BUNCH_WIN):i);
}

bool Paohuzi::comparePending(std::shared_ptr<Game> game,Game::pending_t& x,Game::pending_t& y){
    auto a=x.bunch.type();
    auto b=y.bunch.type();

    if(a<pb_enum::BUNCH_WIN||b<pb_enum::BUNCH_WIN){
        auto of0=fixOps(a);
        auto of1=fixOps(b);
        if((of0==pb_enum::PHZ_AAAwei||of0==pb_enum::PHZ_AAAchou||of0==pb_enum::PHZ_AAAAdesk||of0==pb_enum::PHZ_AAAA)&&b>pb_enum::BUNCH_WIN)
            return true;
        else if((of1==pb_enum::PHZ_AAAwei||of1==pb_enum::PHZ_AAAchou||of1==pb_enum::PHZ_AAAAdesk||of1==pb_enum::PHZ_AAAA)&&a>pb_enum::BUNCH_WIN)
            return false;
        auto o0=opWeight(a);
        auto o1=opWeight(b);
        return o0 > o1;
    }
    //同级别或胡牌情况的优先级处理
    auto M=MaxPlayer();
    auto startIndex=(game->token+1)%M;//记录当前打牌人的下一个位置，作为开始位置,兼容四人玩法
    auto p=x.bunch.pos();
    auto q=y.bunch.pos();
    if(p==game->token)
        return true;	//本人
    else if(q==game->token)
        return false;
    else if(p==(game->token+1)%M)
        return true;	//下手
    else if(q==(game->token+1)%M)
        return false;	//下手
    else if(M==4&&p==(startIndex+1)%M)
        return false;	//四人玩法中的下下手
    else if(M==4&&q==(startIndex+1)%M)
        return false;	//四人玩法的下下手
    else
        return true;
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


