//
//  Mahjong.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "../stdafx.h"
#include "NodeFwd.h"
using namespace proto3;

int Mahjong::Type(){
    return pb_enum::GAME_MJ;
}

int Mahjong::MaxPlayer(){
    return 4;
}

int Mahjong::MaxCards(){
    return 108;
}

int Mahjong::MaxHands(){
    return 17;
}

int Mahjong::Bottom(){
    return 1;
}

void Mahjong::initCard(Game& game){
    unit_id_t id=0;
    //ids => 1111...9999aaaa...iiiiAAAA...III
    for(int i=0;i<3;++i){           //Tong,Suo,Wan
        for(int j=1;j<=9;++j){      //1-9
            for(int k=0;k<4;++k){   //xxxx
                game.pile[id]=id;
                auto& u=game.units[id];
                u.set_color(i);     //Tong,Suo,Wan => 0-3
                u.set_value(i);
                u.set_id(id++);
            }
        }
    }
}

void Mahjong::OnDiscard(Player& player,MsgCNDiscard& msg){
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
        msg.mutable_bunch()->set_pos(player.pos);
        
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
            for(auto h:player.gameData.hands()){
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
        
        std::string str;
        cards2str(*game,str,msg.bunch().pawns());
        KEYE_LOG("OnDiscard pos=%d,cards %s\n",player.pos,str.c_str());
        //remove hands
        auto& hands=*player.gameData.mutable_hands();
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
        //hints
        for(int i=0;i<MaxPlayer();++i){
            auto p=game->players[i];
            auto hints=omsg.mutable_hints();
            hints->Clear();
            if(i!=player.pos){
                google::protobuf::RepeatedField<proto3::bunch_t> bunches;
                if(Hint(bunches,*game,i,*msg.mutable_bunch()))
                    for(auto& b:bunches)hints->Add()->CopyFrom(b);
            }
            p->send(omsg);
        }
        
        //historic
        game->historical.push_back(msg.bunch());
    }else
        player.send(omsg);
}

void Mahjong::OnMeld(Game& game,Player& player,const proto3::bunch_t& bunch){
    if(game.state!=Game::State::ST_MELD){
        KEYE_LOG("OnMeld wrong st=%d",game.state);
        return;
    }
    
    auto pos=player.pos;
    if(bunch.pawns().empty()){
        KEYE_LOG("OnMeld empty cards,pos=%d",pos);
        return;
    }
    
    auto card=*bunch.pawns().rbegin();
    auto& queue=game.pendingMeld;
    if(queue.empty()){
        meld(game,card,game.token);
        KEYE_LOG("OnMeld with queue empty");
        return;
    }
    
    bool found=false;
    int i=0;
    for(;i<queue.size();++i)
        if(pos==queue[i].pos){
            found=true;
            if(card!=queue[i].card){
                KEYE_LOG("OnMeld wrong card=%d,need=%d",queue[i].card,card);
                return;
            }
            break;
        }
    if(!found){
        KEYE_LOG("OnMeld failed: pos=%d, wrong player",pos);
        return;
    }
    std::string str;
    KEYE_LOG("OnMeld pos=%d, ops=%s",pos,bunch2str(game,str,bunch));
    
    bool finish=false;
    bool valid=false;
    if(bunch.type() >= pb_enum::BUNCH_WIN){
        valid=true;
    } else if(bunch.type()==pb_enum::BUNCH_INVALID){
        //invalid
        KEYE_LOG("OnMeld error: unknown ops, pos=%d",pos);
    } else if(bunch.type()==pb_enum::OP_PASS){
        //handle pass
        if(!bunch.pawns().empty()){
        }
    } else{
        //有操作，校验
        auto old_ops=bunch.type();
        /*
        auto res=PreSuit(user,suite).ops;
        if(res!=suite_t::eOps::UNKNOWN&&res!=suite_t::eOps::PASS)
            valid=true;
        else
            KEYE_LOG("OnMeld valid failed: unknown suit=%s, old_ops=%s, pos=%d",suite2String(suite).c_str(),ops2String(old_ops).c_str(),pos);
        */
        valid=true;
    }
    
    //数据放入队列
    queue[i].arrived=true;
    /*
    queue[i].suite=suite;
    
    std::sort(queue.begin(),queue.end(),std::bind(&Mahjong::OpsPred,this,std::placeholders::_1,std::placeholders::_2));
    */
    //依优先级检查玩家操作
    auto& q=queue.front();
    if(q.arrived){
        //clear to avoid duplicated meld
        for(auto w=queue.begin(),ww=queue.end();w!=ww;++w)w->card=-1;
        q.card=card;
        meld(game,card,pos);
    }
}

void Mahjong::meld(Game& game,unit_id_t card,pos_t pos){
    auto& queue=game.pendingMeld;
    //可以返回操作了, GO
    MsgNCMeld msg;
    auto passToken=false;
    auto pass=false;
    auto bunch=msg.mutable_bunch();
    bunch->set_pos(pos);			//当前玩家
    bunch->set_type(pb_enum::BUNCH_INVALID);
    //use test
    /*
    bool bDraw=_desk->deskCardsMap.find(card)!=_desk->deskCardsMap.end();
    if(!queue.empty()){
        //有操作: 偎提碰跑吃胡
        auto& pr=queue.front();
        auto& suite=pr.suite;
        pos=pr.pos;
        auto user=_desk->m_user[pos];
        
        KEYE_LOG("DoAccept pos=%d, ops=%s, card=%d",pos,ops2String(suite.ops).c_str(),msg.m_card);
        
        auto res=suite_t::eOps::UNKNOWN;
        if(suite.ops>=suite_t::eOps::WIN){
            res=suite.ops;
            if(user->handCards.size()==1)
                user->lastHand=true;
            if(suite_t::eOps::AA==fixOps(suite.ops)){
                user->handCards.push_back(suite.cards.back());
            } else{
                res=Suit(user,suite).ops;
            }
            if(res!=suite_t::eOps::UNKNOWN){
                std::vector<suite_t> outSuites;
                if(IsGameOver(pos,outSuites,card)){
                    settle(pos,outSuites,card);
                    PassToken(pos);
                    ChangeState(State::ST_SETTLE);
                }
            } else{
                //诈和
            }
            //进张
            if(_desk->_state!=State::ST_SETTLE)++user->inputCount;
        } else if(suite.ops==suite_t::eOps::PASS){
            //use test
            KEYE_LOG("Pass 2 pos=%d card=%d:%d bDraw=%d",_desk->_token,card,_desk->allCards[card].value,bDraw?1:0);
            //所有人都过
            pass=true;
            //弃牌
            auto owner=_desk->m_user[_desk->_token];
            owner->discardedCards.push_back(card);
            ChangeState(State::ST_DRAW);
            //客户端要求位置为抓牌打牌人的位置
            msg.m_token=_desk->_token;
        } else if(suite.ops==suite_t::eOps::UNKNOWN){
            KEYE_LOG("accept error: unknown ops from pos %d",pos);
            _desk->deskCards.clear();
            Dismiss();
        } else{
            res=Suit(user,suite).ops;
            res=fixOps(res);
            
            //有吃碰，进张
            ++user->inputCount;
            PassToken(pos);	//给吃牌的人
            msg.m_token=_desk->_token;
            //if(res==suite_t::eOps::BBB_B||suite_t::eOps::AAA_A==res||suite_t::eOps::AAAA==res
            //||(user->inputCount==1&&pos!=_desk->_banker)){
            if(res==suite_t::eOps::BBB_B||suite_t::eOps::AAA_A==res||suite_t::eOps::_AAAA==res||suite_t::eOps::AAAA==res
               ||(user->inputCount==1&&pos!=_desk->_banker)){
                //重跑检测
                msg.m_foakDouble=FoakDouble(user);
                if(msg.m_foakDouble){
                    //下一家抓牌
                    KEYE_LOG("accept: foakDouble at %d",pos);
                }
            }
            if(res==suite_t::eOps::ABC||res==suite_t::eOps::AaA){
                //摆火
                for(auto i=user->baihuo.begin(),ii=user->baihuo.end();i!=ii;++i){
                    Suit(user,*i);
                    msg.m_extra.push_back(CardSuite());
                    auto& ex=msg.m_extra.back();
                    ex.type=i->ops;
                    ex.cards.resize(i->cards.size());
                    std::copy(i->cards.begin(),i->cards.end(),ex.cards.begin());
                    KEYE_LOG("accept: sent baihuo pos=%d, ops=%s",pos,suite2String(*i).c_str());
                }
            };
        }
        
        if(!suite.cards.empty())
            msg.m_card=suite.cards.back();
        msg.m_ops.type=suite.ops;
        std::copy(suite.cards.begin(),suite.cards.end(),std::back_inserter(msg.m_ops.cards));
        //video
        if(!suite.cards.empty()){
            auto& m_video=_desk->m_video;
            std::vector<CardValue> cards;
            for(auto it=suite.cards.begin(),iend=suite.cards.end(); it!=iend; ++it){
                cards.push_back(CardValue());
                auto& cvt=_desk->allCards[*it];
                auto& cv=cards.back();
                cv.m_id=cvt.id;
                cv.m_color=cvt.small;
                cv.m_number=cvt.value;
            }
            //use ops as operation
            m_video.AddOper(suite.ops,msg.m_token,cards);
        }
        KEYE_LOG("DoAccept after pos=%d, ops=%s, card=%d",pos,ops2String(suite.ops).c_str(),msg.m_card);
    }else if(pos!=pos_n){
        KEYE_LOG("Pass 0 pos=%d card=%d:%d bDraw=%d",pos,card,_desk->allCards[card].value,bDraw?1:0);
        //弃牌
        auto owner=_desk->m_user[pos];
        owner->discardedCards.push_back(card);
    }
    
    //broadcast
    auto M=Mahjong::maxPlayers(_desk->_gameRule);
    for(Lint i=0; i < M; ++i){
        auto useri=_desk->m_user[i];
        useri->Send(msg);
        useri->spLastMsg.reset(LMsg::Clone(msg));
    }
    
    //结算了不用再抓牌打牌
    if(_desk->_state!=State::ST_SETTLE){
        //八快，PASS,要不起都抓牌
        if(pass||queue.empty()||msg.m_foakDouble){
            if(_desk->deskCards.empty()){
                //荒庄
                KEYE_LOG("IsGameOver no cards on desk");
                Dismiss();
            } else
                draw();
        } else
            checkAndDiscsard(_desk->_token,pos_n);
    }
    */
}

void Mahjong::draw(Game& game){
    Next(game);
    auto player=game.players[game.token];
    auto card=game.pile.back();
    game.pile.pop_back();
    ChangeState(game,Game::State::ST_DISCARD);
    game.pendingDiscard=std::make_shared<Game::pending_t>();
    game.pendingDiscard->card=card;
    game.pendingDiscard->pos=game.token;
    MsgCNDiscard msg;
    msg.mutable_bunch()->add_pawns(card);
    OnDiscard(*player,msg);
}

bool Mahjong::isNaturalWin(Game& game,pos_t pos){
    return false;
}

void Mahjong::PostTick(Game& game){
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
                        auto& gdata=robot->gameData;
                        auto& hands=gdata.hands();
                        if(hands.empty()){
                            break;
                        }else{
                            auto& B=game.units[gdata.selected_card()];
                            auto it=hands.rbegin();
                            //find huazhu if exists
                            for(auto iter=hands.rbegin(),iend=hands.rend();iter!=iend;++iter){
                                auto& A=game.units[*iter];
                                if(A.color()==B.color()){
                                    it=iter;
                                    break;
                                }
                            }
                            msg.mutable_bunch()->add_pawns(*it);
                            OnDiscard(*robot,msg);
                        }
                        game.delay=0;
                    }
                }
                break;
            case Game::State::ST_MELD:
                //show card and wait for accept
                if(!game.pendingMeld.empty()){
                    for(auto i=game.pendingMeld.begin(),ii=game.pendingMeld.end(); i!=ii; ++i)
                        if(i->pos==robot->pos){
                            if(i->arrived)
                                //already processed
                                break;
                            auto pmsg=robot->lastMsg.get();
                            if(auto msg=dynamic_cast<MsgNCDiscard*>(pmsg)){
                                auto& hints=msg->hints();
                                if(!hints.empty()){
                                    auto& bunch=hints.Get(0);
                                    OnMeld(game,*robot,bunch);
                                    //KEYE_LOG("ProcessRobot st=%s, pos=%d, suite=%s",st2String(st).c_str(),pos,suite2String(suite).c_str());
                                } else {
                                    //KEYE_LOG("ProcessRobot st=%s, pos=%d, no suite found",st2String(st).c_str(),pos);
                                }
                            }
                            break;
                        }
                }
                break;
            default:
                break;
        }
    }
}

bool Mahjong::Settle(Game& game){
    pos_t pos=-1;
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto& gd=game.players[i]->gameData;
        if(gd.hands().size()<=0)
            pos=i;
    }

    //broadcast
    MsgNCSettle msg;
    msg.set_mid(pb_msg::MSG_NC_SETTLE);
    msg.set_winner(pos);
    for(uint i=0,ii=MaxPlayer();i!=ii;++i){
        auto hand=msg.add_hands();
        hand->mutable_pawns()->CopyFrom(game.players[i]->gameData.hands());
        //auto player=msg.add_play();
    }
    
    for(auto p:game.players){
        p->send(msg);
        p->ready=false;
    }
    
    if(++game.round>=game.Round){
        MsgNCFinish fin;
        fin.set_mid(pb_msg::MSG_NC_FINISH);
        fin.set_result(pb_enum::SUCCEESS);
        for(auto p:game.players)p->send(msg);
        return true;
    }
    return false;
}

bool Mahjong::IsGameOver(Game& game){
    for(auto player:game.players){
        if(player->gameData.hands().size()<=0)
            return true;
    }
    return false;
}

bool Mahjong::Hint(google::protobuf::RepeatedField<bunch_t>& bunches,Game& game,pos_t pos,proto3::bunch_t& bunch){
    auto& hands=game.players[pos]->gameData.hands();
    return bunches.size()>0;
}

pb_enum Mahjong::verifyBunch(Game& game,bunch_t& bunch){
    auto bt=pb_enum::BUNCH_A;
    do{
        if(bunch.pawns_size()<=0){
            bt=pb_enum::BUNCH_INVALID;
            break;
        }
        auto& gdata=game.players[bunch.pos()]->gameData;
        auto& A=game.units[bunch.pawns(0)];
        if(gdata.selected_card()!=i_invalid){
            //huazhu
            auto& B=game.units[gdata.selected_card()];
            if(A.color()!=B.color()){
                bt=pb_enum::BUNCH_INVALID;
                break;
            }
        }
    }while (false);
    bunch.set_type(bt);
    return bt;
}

bool Mahjong::compareBunch(Game& game,bunch_t& bunch,bunch_t& hist){
    return true;
}

bool Mahjong::comparision(Game& game,uint x,uint y){
    auto cx=game.units[x];
    auto cy=game.units[y];
    return cx.value()<cy.value();
}

void Mahjong::make_bunch(Game& game,proto3::bunch_t& bunch,const std::vector<uint>& vals){
    bunch.mutable_pawns()->Clear();
    for(auto n:vals){
        uint color=n/100;
        uint val=n%100;
        uint id=0;
        for(auto card:game.units)if(card.value()==val&&card.color()==color)id=card.id();
        bunch.mutable_pawns()->Add(id);
    }
}

void Mahjong::test(){
    Mahjong ddz;
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


