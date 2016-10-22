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

void Paohuzi::Tick(Game& game){
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
            //OnDiscard
            break;
        case Game::State::ST_MELD:
            //OnMeld
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

void Paohuzi::OnDiscard(Player& player,MsgCNDiscard& msg){
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
        
        //cards check
        auto card=(unit_id_t)msg.bunch().pawns(0);
        //boundary check
        if(!validId(card)){
            KEYE_LOG("OnDiscard invalid cards %d\n",card);
            break;
        }

        //shut discard after verify
        if(!game->pendingDiscard){
            KEYE_LOG("OnDiscard not on pending\n");
            break;
        }else
            player.game->pendingDiscard.reset();

        std::string str;
        cards2str(str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*player.playData.mutable_hands();
        for(auto j:msg.bunch().pawns()){
            for(auto i=hands.begin();i!=hands.end();++i){
                if(j==*i){
                    //KEYE_LOG("OnDiscard pos=%d,erase card %d\n",player.pos,*i);
                    hands.erase(i);
                    break;
                }
            }
        }
        //logHands(*game,player.pos,"OnDiscard");
        omsg.set_result(pb_enum::SUCCEESS);
        omsg.mutable_bunch()->CopyFrom(msg.bunch());

        //game->pendingMeld.clear();
        omsg.mutable_bunch()->set_pos(player.pos);
        
        //ready for meld
        changeState(*player.game,Game::State::ST_MELD);
        //pending meld
        for(int i=0;i<MaxPlayer();++i){
            auto p=game->players[i];
            if(i!=player.pos){
                //only pending others
                game->pendingMeld.push_back(Game::pending_t());
                auto& pending=game->pendingMeld.back();
                pending.bunch.set_pos(i);
                pending.bunch.add_pawns(card);
            }
            p->send(omsg);
            p->lastMsg=std::make_shared<MsgNCDiscard>(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
        return;
    }while(false);
    player.send(omsg);
}

void Paohuzi::OnMeld(Player& player,const proto3::bunch_t& curr){
    auto pos=player.pos;
    auto spgame=player.game;
    if(!spgame){
        KEYE_LOG("OnMeld no game\n");
        return;
    }
    auto& game=*spgame;
    //state
    if(game.state!=Game::State::ST_MELD){
        KEYE_LOG("OnMeld wrong st=%d,pos=%d\n",game.state,pos);
        return;
    }
    
    //pending queue
    auto& pendingMeld=game.pendingMeld;
    if(pendingMeld.empty()){
        KEYE_LOG("OnMeld with queue empty,pos=%d\n",pos);
        return;
    }

    //pos
    bool found=false;
    int i=0;
    for(;i<pendingMeld.size();++i)
        if(pos==pendingMeld[i].bunch.pos()){
            found=true;
            break;
        }
    if(!found){
        KEYE_LOG("OnMeld wrong player pos=%d\n",pos);
        return;
    }
    
    //arrived and duplicated
    auto& pending=pendingMeld[i];
    if(pending.arrived){
        KEYE_LOG("OnMeld already arrived, pos=%d\n",pos);
        return;
    }
    pending.arrived=true;

    if(pending.bunch.pawns_size()<=0){
        KEYE_LOG("OnMeld empty cards,pos=%d\n",pos);
        return;
    }

    //card or just pass
    int card=-1;
    if(curr.type()!=pb_enum::OP_PASS){
        if(curr.pawns().empty()){
            KEYE_LOG("OnMeld empty cards,pos=%d\n",pos);
            return;
        }
        card=*curr.pawns().rbegin();
        
        auto pcard=*pending.bunch.pawns().rbegin();
        if(card!=pcard){
            KEYE_LOG("OnMeld wrong card=%d,need=%d,pos=%d\n",pcard,card,pos);
            return;
        }
    }

    //queue in
    std::string str;
    //KEYE_LOG("OnMeld queue in,pos=%d,%s\n",pos,bunch2str(str,curr));
    pending.bunch.CopyFrom(curr);
    
    int ready=0;
    for(auto& p:pendingMeld)if(p.arrived)++ready;
    if(ready>=pendingMeld.size()){
        //sort
        std::sort(pendingMeld.begin(),pendingMeld.end(),std::bind(&Paohuzi::comparePending,this,std::placeholders::_1,std::placeholders::_2));
        
        //priority
        auto& front=pendingMeld.front();
        auto& bunch=front.bunch;

        //ok,verify
        MsgNCMeld msg;
        msg.set_mid(pb_msg::MSG_NC_MELD);
        msg.set_result(pb_enum::SUCCEESS);
        KEYE_LOG("OnMeld pos=%d,%s,token=%d\n",pos,bunch2str(str,bunch),game.token);

        auto isDraw=(pendingMeld.size()==1);
        switch(bunch.type()){
            case pb_enum::BUNCH_WIN:{
                std::vector<bunch_t> output;
                if(isGameOver(game,pos,card,output)){
                    player.playData.clear_hands();
                }
                break;
            }
            case pb_enum::OP_PASS:
                //handle pass, ensure token
                if(isDraw)
                    bunch.set_pos(game.token);
                else
                    bunch.set_pos(-1);
                break;
            case pb_enum::BUNCH_INVALID:
                //invalid
                KEYE_LOG("OnMeld error, unknown ops, pos=%d\n",pos);
                msg.set_result(pb_enum::BUNCH_INVALID);
                break;
            case pb_enum::BUNCH_A:
                //collect after draw
                player.playData.mutable_hands()->Add(card);
                //pending discard
                game.pendingDiscard=std::make_shared<Game::pending_t>();
                game.pendingDiscard->bunch.set_pos(player.pos);
                //remove from pile map
                game.pileMap.erase(card);
                break;
            default:{
                //verify
                auto old_ops=bunch.type();
                auto result=verifyBunch(game,*(bunch_t*)&bunch);
                if(result==pb_enum::BUNCH_INVALID){
                    std::string str;
                    KEYE_LOG("OnMeld verify failed,bunch=%s, old_ops=%d, pos=%d\n",bunch2str(str,bunch),old_ops,pos);
                    msg.set_result(pb_enum::BUNCH_INVALID);
                }else{
                    //erase from hands
                    auto& hands=*player.playData.mutable_hands();
                    for(auto j:bunch.pawns()){
                        for(auto i=hands.begin();i!=hands.end();++i){
                            if(j==*i){
                                //KEYE_LOG("OnMeld pos=%d,erase card %d\n",player.pos,*i);
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
                    game.pendingDiscard->bunch.set_pos(player.pos);
                    changePos(game,player.pos);
                }//meld
            }//default
        }//switch

        //change state before send message
        auto needDraw=false;
        if(bunch.type()==pb_enum::OP_PASS){
            if(isDraw){
                //draw pass to discard
                //KEYE_LOG("OnMeld pass to discard\n");
                changeState(game,Game::State::ST_DISCARD);
                //pending discard
                game.pendingDiscard=std::make_shared<Game::pending_t>();
                game.pendingDiscard->bunch.set_pos(game.token);
            }else{
                //discard pass to draw,don't do it immediately!
                needDraw=true;
            }
        }else if(GameRule::isGameOver(game))
            changeState(game,Game::State::ST_SETTLE);
        else //A,AAA,AAAA
            changeState(game,Game::State::ST_DISCARD);

        game.pendingMeld.clear();

        msg.mutable_bunch()->CopyFrom(bunch);
        for(auto p:game.players){
            //need reply all
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCMeld>(msg);
        }
        
        //then draw
        if(needDraw){
            //KEYE_LOG("OnMeld pass to draw\n");
            draw(game);
            changeState(game,Game::State::ST_MELD);
        }
    }//if(ready>=queue.size())
}

void Paohuzi::draw(Game& game){
    changePos(game,game.token+1);
    auto player=game.players[game.token];
    auto card=game.pile.back();
    game.pile.pop_back();
    KEYE_LOG("draw pos=%d, card %d\n",game.token,card);

    //game.pendingMeld.clear();
    game.pendingMeld.push_back(Game::pending_t());

    MsgNCDraw msg;
    msg.set_mid(pb_msg::MSG_NC_DRAW);
    msg.set_pos(game.token);
    for(int i=0;i<MaxPlayer();++i){
        auto p=game.players[i];
        if(i==game.token){
            msg.set_card(card);

            //pending meld
            auto& pending=game.pendingMeld.back();
            pending.bunch.set_pos(game.token);
            pending.bunch.add_pawns(card);
        }else{
            msg.set_card(card);
//            msg.set_card(-1);
        }
        p->send(msg);
        p->lastMsg=std::make_shared<MsgNCDraw>(msg);
    }
}

bool Paohuzi::isNaturalWin(Game& game,pos_t pos){
    return false;
}

bool Paohuzi::settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.players[i]->playData;
        if(gd.hands().size()<=0)
            pos=i;
    }

    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto play=msg.mutable_play(i);
        play->set_win(i==pos?1:0);
        play->mutable_hands()->CopyFrom(game.players[i]->playData.hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players){
        p->send(msg);
        p->lastMsg=std::make_shared<MsgNCSettle>(msg);
    }
    
    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players){
            p->send(msg);
            p->lastMsg=std::make_shared<MsgNCFinish>(fin);
        }
        return true;
    }
    return false;
}

bool Paohuzi::isGameOver(Game& game,pos_t pos,unit_id_t card,std::vector<proto3::bunch_t>& output){
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
    std::vector<int> hand(hands.begin(),hands.end());
    
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
            std::vector<int> tmpHand;
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

bool Paohuzi::isGameOver(Game& game,std::vector<int>& cards,std::vector<bunch_t>& allSuites){
    //recursive check if is game over
    std::vector<bunch_t> outSuites;
    std::vector<bunch_t> multiSuites;
    //copy hand
    std::vector<int> _cards(cards);
    
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
            std::vector<int> mcards(_cards);
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

void Paohuzi::hint(Game& game,unit_id_t card,std::vector<int>& _hand,std::vector<bunch_t>& hints){
    //这张牌可能的所有组合：句,绞
    std::vector<int> jiao,same;
    auto A=card;
    
    std::vector<int> hand(_hand);
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

bool Paohuzi::hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& src_bunch){
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
    switch (bunch.type()) {
        case pb_enum::BUNCH_A:
            if(bunch.pawns_size()==1)
                bt=bunch.type();
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
        default:
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

bool Paohuzi::comparision(uint x,uint y){
    auto cx=x/1000;
    auto cy=y/1000;
    if(cx<cy)return true;
    else if(cx==cy)return x%100<y%100;
    else return false;
}

bool Paohuzi::comparePending(Game::pending_t& x,Game::pending_t& y){
    auto a=(int)x.bunch.type();
    auto b=(int)y.bunch.type();
    return a>b;
}
int Paohuzi::winPoint(Game&,proto3::pb_enum){
    return 1;
}
int Paohuzi::calcScore(Game&,proto3::pb_enum,int points){
    return 1;
}
int Paohuzi::calcPoints(Game&,std::vector<proto3::bunch_t>&){
    return 1;
}
int Paohuzi::calcPoints(Game&,pos_t){
    return 1;
}
                               bool Paohuzi::chouWei(Game&,pos_t,proto3::bunch_t&){
                                   return true;
                               }

void Paohuzi::test(){
    Paohuzi ddz;
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


