// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ImmortalFwd.h"

#ifdef WIN32
#include <conio.h>
#else
#include <unistd.h>
#endif

using namespace keye;
using namespace proto3;

enum TIMER:size_t{
    TIMER_SEC=100,
    TIMER_MIN,
    TIMER_HOUR,
    TIMER_DAY,
};

#if defined(WIN32) || defined(__APPLE__)
keye::logger sLogger;
#else
keye::logger sLogger("immortal.log");
#endif

Immortal* Immortal::sImmortal=nullptr;

Immortal::Immortal(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size)
,_game_index(0){
    sImmortal=this;
}

void Immortal::registerRule(std::shared_ptr<GameRule> game){
    auto id=game->Type();
    gameRules[id]=game;
}

std::shared_ptr<Game> Immortal::createGame(int key,proto3::MsgCNCreate& msg){
    std::shared_ptr<Game> gameptr;
    auto rule=gameRules.find(msg.game());
    if(rule!=gameRules.end()&&key>0){
        auto gid=key*pb_enum::DEF_MAX_GAMES_PER_NODE+_game_index++;
        gameptr=std::make_shared<Game>();
        games[gid]=gameptr;
        gameptr->id=gid;
        gameptr->rule=rule->second;
    }else
        Logger<<"create game error no rule "<<msg.game()<<endl;
    return gameptr;
}

std::shared_ptr<Game> Immortal::findGame(game_id_t id){
    return games.count(id)>0?games[id]:std::shared_ptr<Game>();
}

void Immortal::removeGame(game_id_t id){
    gameRules.erase(id);
}

void Immortal::addPlayer(size_t shid,std::shared_ptr<Player> sp){
    if(!sp)
        return;
    players[shid]=sp;;
}

void Immortal::on_open(svc_handler&) {
    Logger<<"on_open\n";
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void Immortal::on_close(svc_handler& sh) {
    auto shid=sh.id();
    players.erase(shid);
    Logger<<"on_close\n";
}

void Immortal::on_read(svc_handler& sh, void* buf, size_t sz) {
    auto shid=sh.id();
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    if(mid==proto3::pb_msg::MSG_CN_CONNECT){
        MsgCNConnect imsg;
        MsgNCConnect omsg;
        if(pb.Parse(imsg)){
            auto spPlayer=std::make_shared<Player>(sh);
            auto player=spPlayer->playData.mutable_player();
            player->set_uid(imsg.uid());
            player->set_level(168);
            player->set_currency(1000);
            addPlayer(shid,spPlayer);

            omsg.set_result(proto3::pb_enum::SUCCEESS);
            omsg.mutable_player()->CopyFrom(*player);
            Logger<<"client entered,uid="<<imsg.uid()<<"\n";
            
        }
        else
        {
            Logger<<"message error id="<<mid<<endl;
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
        }
        omsg.set_mid(proto3::pb_msg::MSG_NC_CONNECT);
        PBHelper::Send(sh,omsg);
    }else{
        auto p=players.find(shid);
        if(p!=players.end())
            p->second->on_read(pb);
    }
    //Logger<<"on_read %zd,mid=%d\n", sz,mid);
}

void Immortal::on_write(svc_handler&, void*, size_t sz) {
//    Logger<<"on_write %zd\n",sz);
}

bool Immortal::on_timer(svc_handler&, size_t id, size_t milliseconds) {
    switch (id) {
        case TIMER::TIMER_SEC:
            for(auto game:games)game.second->rule->Tick(*game.second);
            break;
        case TIMER::TIMER_MIN:
            break;
        case TIMER::TIMER_HOUR:
            break;
        case TIMER::TIMER_DAY:
            break;
        default:
            break;
    }
    return true;
}
