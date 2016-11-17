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

int Paohuzi::MaxPlayer(Game& game){
    switch (game.category) {
        case pb_enum::PHZ_HY:
        case pb_enum::PHZ_PEGHZ:
            return 4;
        default:
            return 3;
    }
}

int Paohuzi::maxCards(Game& game){
    return 80;
}

int Paohuzi::maxHands(Game& game){
    switch (game.category) {
        case pb_enum::PHZ_HY:
        case pb_enum::PHZ_PEGHZ:
            return 14;
        default:
            return 20;
    }
}

int Paohuzi::bottom(Game& game){
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
            auto bunch=msg.mutable_bunch(p->playData.seat());
            bunch->set_pos(p->playData.seat());
            bunch->set_type(pb_enum::PHZ_AAAAstart);
            for(auto& aaaa:p->AAAAs)
                for(auto c:aaaa.pawns())bunch->add_pawns(c);
        }
    }
    
    MeldGame::engage(game,msg);
}

void Paohuzi::draw(Game& game){
    auto card=game.pile.empty()?invalid_card:game.pile.back();

    MeldGame::draw(game);
    
    if(card!=invalid_card){
        //no pending meld,just pending discard
        checkDiscard(*game.players[game.token],card);
        game.pendingMeld.clear();
    }
}

bool Paohuzi::meld(Game& game,Player& player,unit_id_t card,bunch_t& bunch){
    //past,dodge and conflict
    auto past=std::find(player.unpairedCards.begin(),player.unpairedCards.end(),card)!=player.unpairedCards.end();
    if(past||player.conflictMeld){
        if(pb_enum::PHZ_ABC==bunch.type()||pb_enum::PHZ_BBB==bunch.type()){
            Logger<<"meld failed, past card or conflict\n";
            return false;
        }
    }
    auto dodge=std::find(player.dodgeCards.begin(),player.dodgeCards.end(),card)!=player.dodgeCards.end();
    if(dodge){
        if(pb_enum::PHZ_BBB==bunch.type()){
            Logger<<"meld failed, dodge card\n";
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
            Logger<<"meld failed, baihuo "<<card<<endl;
            return false;
        }
    }
    
    //erase from hands
    auto& hands=*player.playData.mutable_hands();
    for(auto j:bunch.pawns()){
        for(auto i=hands.begin();i!=hands.end();++i){
            if(j==*i){
                //Logger<<"OnMeld pos=%d,erase card %d\n",where,*i);
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
        case pb_enum::PHZ_B4B3:
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
    return true;
}

void Paohuzi::onMeld(Game& game,Player& player,unit_id_t card,proto3::bunch_t& bunch){
    //remove all past cards,deal it below
    auto pos=player.playData.seat();
    for(auto pm:game.pendingMeld){
        auto i=pm.bunch.pos();
        if(i==-1)i=pos;
        auto p=game.players[i];
        p->unpairedCards.resize(std::remove(p->unpairedCards.begin(),p->unpairedCards.end(),card)-p->unpairedCards.begin());
    }

    switch (bunch.type()) {
        case proto3::OP_PASS:{
            //means all players passed,should mark all of them
            //clients bring all hints even pass,handle here
            for(auto pm:game.pendingMeld){
                auto dodge=false,past=false;
                auto i=pm.bunch.pos();
                if(i==-1)i=pos;
                for(auto child:pm.bunch.child()){
                    auto type=child.type();
                    if(type==pb_enum::PHZ_ABC){
                        past=true;
                    }
                    if(type==pb_enum::PHZ_BBB){
                        dodge=true;
                    }
                }
                
                //remember past and dodge cards
                if(dodge){
                    game.players[i]->dodgeCards.push_back(card);

                    auto chBunch=bunch.mutable_child()->Add();
                    chBunch->set_pos(i);
                    chBunch->set_type(pb_enum::PHZ_BBB);
                    Logger<<dodge<<" dodge "<<card<<endl;
                }else if(past){
                    game.players[i]->unpairedCards.push_back(card);
                    
                    auto chBunch=bunch.mutable_child()->Add();
                    chBunch->set_pos(i);
                    chBunch->set_type(pb_enum::PHZ_ABC);
                    Logger<<past<<" pass meld "<<card<<endl;
                }
            }
   
            break;
        }
        case proto3::PHZ_ABC:{
            //still have somebody pass!!
            auto hasPass=false;
            for(auto pm:game.pendingMeld){
                if(pm.bunch.type()==pb_enum::OP_PASS){
                    hasPass=true;
                    break;
                }
            }
            if(!hasPass)break;

            std::vector<int> pasts(MaxPlayer(game));
            for(auto it=game.pendingMeld.begin();it!=game.pendingMeld.end();++it){
                auto& pm=*it;
                if(pm.bunch.type()==pb_enum::OP_PASS){
                    auto dodge=false;
                    auto i=pm.bunch.pos();
                    if(i==-1)i=pos;
                    for(auto child:pm.bunch.child()){
                        auto type=child.type();
                        if(type==pb_enum::PHZ_ABC){
                            pasts[i]=1;
                        }
                        if(type==pb_enum::PHZ_BBB){
                            dodge=true;
                        }
                    }
                    //handle dodge directly
                    if(dodge){
                        pasts[i]=0;
                        game.players[i]->dodgeCards.push_back(card);
                        
                        auto chBunch=bunch.mutable_child()->Add();
                        chBunch->set_pos(i);
                        chBunch->set_type(pb_enum::PHZ_BBB);
                        Logger<<i<<" dodge "<<card<<endl;
                    }
                }
            }
            
            //then find the highest one to be the responsible
            for(int i=game.token;i<game.token+MaxPlayer(game);++i){
                auto j=i%MaxPlayer(game);
                if(pasts[j]==0)
                    break;
                
                //else all of then were pass
                game.players[j]->unpairedCards.push_back(card);
                
                bunch.mutable_child()->Add()->set_pos(j);
                bunch.mutable_child()->Add()->set_type(pb_enum::PHZ_ABC);
                Logger<<j<<" past meld "<<card<<endl;
            }
            break;
        }
        case proto3::PHZ_BBBBdesk:
            //remember conflict meld
            if(game.token!=pos && game.pileMap.find(card)==game.pileMap.end()){
                game.players[game.token]->conflictMeld=true;
                Logger<<game.token<<" conflict "<<card<<endl;
            }
            break;
        default:
            break;
    }
}

bool Paohuzi::isWin(Game& game,proto3::bunch_t& bunch,std::vector<proto3::bunch_t>& output){
    auto M=MaxPlayer(game);
    int MIN_SUITES=7;
    if(M==4)MIN_SUITES=5;
    if(bunch.type()<pb_enum::BUNCH_WIN || bunch.child_size()<MIN_SUITES){
        Logger<<"isWin failed: wrong bunch size "<<bunch.child_size()<<endl;
        return false;
    }
    
    auto pos=bunch.pos();
    auto card=bunch.pawns(0);
    auto& player=*game.players[pos];
    auto& suite=*player.playData.mutable_bunch();
    auto& hands=*player.playData.mutable_hands();
    
    //logHands(game,pos);

    //can't win hand card if not fire
    auto bDraw=(!validId(card)||game.pileMap.find(card)!=game.pileMap.end()||game.firstCard==card);
    auto fire=(pos!=game.token && !bDraw
               &&   (game.category==pb_enum::PHZ_LD||game.category==pb_enum::PHZ_HY||
                     game.category==pb_enum::PHZ_XX_GHZ||game.category==pb_enum::PHZ_CZ||
                     game.category==pb_enum::PHZ_HY||game.category==pb_enum::PHZ_GX));
    if(!bDraw && !fire){
        Logger<<"isWin failed: not fire and not from pile\n";
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
                Logger<<"isWin failed: card "<<c<<" not exists\n";
                return false;
            }
        }
    }
    for(auto& kv:cmap)if(kv.second!=0){
        Logger<<"isWin failed: card "<<kv.first<<" missing\n";
        return false;
    }
    
    //verify bunch
    for(auto& b:*bunch.mutable_child()){
        if(pb_enum::BUNCH_INVALID==verifyBunch(game,b)){
            std::string str;
            Logger<<"isWin failed: invalid bunch "<<bunch2str(str,b)<<endl;
            return false;
        }
    }

    std::copy(bunch.child().begin(),bunch.child().end(),std::back_inserter(output));
    auto point=calcPoints(game,output);
    if(point<winPoint(game,game.category)){
        Logger<<"isWin failed: not enough points "<<point<<endl;
        output.clear();
        return false;
    }
    return true;
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
        case pb_enum::PHZ_B4B3:
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
        case pb_enum::BUNCH_WIN:
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
    auto pos=player.playData.seat();
    auto& game=*player.game;
    auto M=MaxPlayer(game);
    std::stringstream ss;//广西跑胡子用
    int _cardNum=0;//广西跑胡子用
    //清洗缓冲区
    ss.clear();
    auto& play=player.playData;
    auto& playFinish=*game.spFinish->mutable_play(pos);

    int point=0,score=0,multiple=0,chunk=0;
    bool self=(pos==game.token&&game.pileMap.find(card)!=game.pileMap.end());
    bool fire=(pos!=game.token&&invalid_card!=card&&game.pileMap.find(card)!=game.pileMap.end()
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
        play.set_win(1);
        
        //先计名堂
        calcAchievement(game,game.category,allSuites,achvs);
        
        //天胡
        auto naturalWin=true;
        if(game.pile.size()<19)
            naturalWin=false;
        for(auto p:game.players){
            if(p->playData.bunch_size()>0 ||
               p->playData.discards_size()>0){
                naturalWin=false;
                break;
            }
        }
        if(naturalWin&&game.category!=pb_enum::PHZ_SY
           &&game.category!=pb_enum::PHZ_CD_HHD&&game.category!=pb_enum::PHZ_HY){
            if(pos==game.banker){
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
            }else{
                //地胡，有时候会检测到放炮的名堂，但是是不允许的，这里在地胡处理中直接屏蔽
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
            playFinish.set_score(playFinish.score()+score);
//            msg.m_scores[pos]=score;
//            spFinalEnd->m_total[pos].score+=score;
            
            if(fire){
                //放炮，两人变动
//                msg.mutable_play(game.token)->set_score(-score);
                game.players[game.token]->playData.set_score(-score);
                game.spFinish->mutable_play(game.token)->set_score(game.spFinish->play(game.token).score()-score);
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
                game.players[game.token]->playData.set_score(-(bMulti3?score*3:score*2));
//                msg.m_scores[pos]=bMulti3?score*3:score*2;
//                msg.m_scores[game.token]=-(bMulti3?score*3:score*2);
            } else{
                //适配4人玩法
                int multi=M-1;
                if(game.category==pb_enum::PHZ_CD_HHD||game.category==pb_enum::PHZ_CD_QMT)
                {
                    score=calcMultiOrScore(game,score*multi);
                    for(int i=0; i<M; ++i){
                        game.players[i]->playData.set_score(i==pos?score:-(score/multi));
//                        msg.m_scores[i]=(i==pos?score:-(score/multi));
                    }
                } else{
                    for(int i=0; i<M; ++i)
                        game.players[i]->playData.set_score(i==pos?score*multi:-score);
//                        msg.m_scores[i]=(i==pos?score*multi:-score);
                }
            }
            for(int i=0; i<M; ++i){
                game.spFinish->mutable_play(i)->set_score(game.spFinish->play(i).score()+game.players[i]->playData.score());
//                spFinalEnd->m_total[i].score+=msg.m_scores[i];
//                msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
            }
        }
    } else if(point>=0){
        if(game.category==pb_enum::PHZ_CZ){
            //郴州毛胡子玩法，荒庄需要换庄
            auto M=MaxPlayer(game);
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
            game.players[game.banker]->playData.set_score(-10);
        for(int i=0; i<M; ++i){
            game.spFinish->mutable_play(i)->set_score(game.spFinish->play(i).score()+game.players[i]->playData.score());
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
            }else if(resSuite==pb_enum::PHZ_BBB_B||resSuite==pb_enum::PHZ_BBBBdesk||resSuite==pb_enum::PHZ_B4B3){
                //跑牌胡
                bool bWei=true;//checkWeiPengAct(suite,pos,card);//true:偎坎牌跑胡，false:碰牌跑胡
                if(resSuite==pb_enum::PHZ_BBB_B){
                    game.players[pos]->m_winMark=pb_enum::WINWPAO;//偎坎跑胡
                } else if(resSuite==pb_enum::PHZ_BBBBdesk||resSuite==pb_enum::PHZ_B4B3){
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
        bool pfire=(pos!=game.token&&card!=invalid_card&&game.pileMap.find(card)==game.pileMap.end()
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
                game.spFinish->mutable_play(i)->set_score(game.spFinish->mutable_play(i)->score()+game.players[i]->playData.score());
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
                game.spFinish->mutable_play(i)->set_score(game.spFinish->play(i).score()+game.players[i]->playData.score());
                //spFinalEnd->m_total[i].score+=msg.m_scores[i];
                //msg.m_totalScores[i]=spFinalEnd->m_total[i].score;
            }
        }
    }//pb_enum::PHZ_PENGHUZI
    
    play.set_point(point);
    play.set_chunk(chunk);
    play.set_multiple(multiple);
    
    //copy hidden bunches after settle
    for(int i=0; i<MaxPlayer(game); ++i){
        auto localPlayer=game.players[i];
        //the loser
        for(auto& aaa:localPlayer->AAAAs)
            localPlayer->playData.add_bunch()->CopyFrom(aaa);
        for(auto& aaa:localPlayer->AAAs)
            localPlayer->playData.add_bunch()->CopyFrom(aaa);
    }
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
        if(iv->type()==pb_enum::PHZ_AAAA||iv->type()==pb_enum::PHZ_AAAAdesk
           ||iv->type()==pb_enum::PHZ_BBB_B||iv->type()==pb_enum::PHZ_BBBBdesk||iv->type()==pb_enum::PHZ_B4B3){
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
            } else if(it->pawns_size()==4&&(resOps==pb_enum::PHZ_AAAA||resOps==pb_enum::PHZ_AAAAstart||resOps==pb_enum::PHZ_AAAAdesk||resOps==pb_enum::PHZ_BBBBdesk||resOps==pb_enum::PHZ_B4B3||resOps==pb_enum::PHZ_BBB_B)){
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

void Paohuzi::sortPendingMeld(std::shared_ptr<Game> spgame,std::vector<proto3::bunch_t>& pending){
    MeldGame::sortPendingMeld(spgame,pending);
    if(spgame->category==pb_enum::PHZ_GX)
        for(auto i=spgame->pendingMeld.begin()+1,iend=spgame->pendingMeld.end();i!=iend;++i){
            if(i->bunch.type()>=pb_enum::BUNCH_WIN)
                pending.push_back(i->bunch);
        }
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
    auto sorter=std::bind(&Paohuzi::comparision,this,std::placeholders::_1,std::placeholders::_2);
    int point=0;
    for(auto i=allSuites.begin(),ii=allSuites.end(); i!=ii; ++i){
        auto& suite=*i;
        if(suite.pawns().empty())continue;
        auto small=(1==suite.pawns(0)/1000);
        int pt=0;
        switch(fixOps(suite.type())){
            case pb_enum::PHZ_AAAA:
            case pb_enum::PHZ_AAAAstart:
            case pb_enum::PHZ_AAAAdesk:
                pt+=(small?9:12);
                break;
            case pb_enum::PHZ_B4B3:
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
                std::sort(sl.begin(),sl.end(),sorter);
                auto A=sl[0];
                auto B=sl[1];
                if(A/1000==B/1000 && (A%100==1 || (A%100==2&&B%100==7)))
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

//封顶统计
int Paohuzi::calcMultiOrScore(Game& game,int value){
    auto gameRule=game.category;
    int res=0;
    auto limit=-1;//_desk->_multiScore;
    if(limit<=0)
        return value;//认为无限制

    switch(gameRule){
        case pb_enum::PHZ_SY:		//邵阳字牌
        case pb_enum::PHZ_SYBP:	//邵阳剥皮
        case pb_enum::PHZ_LD:		//娄底放炮
        case pb_enum::PHZ_HY:		//衡阳六条枪
        default:
            res=value;
            break;
        case pb_enum::PHZ_CD_QMT:	//常德全名堂
        case pb_enum::PHZ_CD_HHD:	//常德红黑点
        case pb_enum::PHZ_HH:		//怀化红拐弯
        case pb_enum::PHZ_CS:		//长沙跑胡子
            if(value>limit){
                res=limit;
            } 
            else{
                res=value;
            }
            break;
    }
    return res;
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
        case pb_enum::PHZ_B4B3:
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
    return (ops>=pb_enum::BUNCH_WIN?i+(ops/pb_enum::BUNCH_WIN*pb_enum::BUNCH_WIN):i);
}

bool Paohuzi::comparePending(std::shared_ptr<Game> game,Game::pending_t& x,Game::pending_t& y){
    auto a=x.bunch.type();
    auto b=y.bunch.type();

    if(a<pb_enum::BUNCH_WIN||b<pb_enum::BUNCH_WIN){
        auto of0=fixOps(a);
        auto of1=fixOps(b);
        if(of0==pb_enum::PHZ_AAAwei||of0==pb_enum::PHZ_AAAchou||of0==pb_enum::PHZ_AAAAdesk||of0==pb_enum::PHZ_AAAA)
            return true;
        else if(of1==pb_enum::PHZ_AAAwei||of1==pb_enum::PHZ_AAAchou||of1==pb_enum::PHZ_AAAAdesk||of1==pb_enum::PHZ_AAAA)
            return false;
        auto o0=opWeight(a);
        auto o1=opWeight(b);
        return o0 > o1;
    }
    //同级别或胡牌情况的优先级处理
    auto M=MaxPlayer(*game);
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


