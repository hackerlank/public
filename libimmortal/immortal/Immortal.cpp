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

void Immortal::run(const char* cfg){
    if(cfg && config.load(cfg)){
        auto port=(short)(int)config.value("port");
        ws_service::run(port,"127.0.0.1");
        Logger<<"server start at "<<port<<endf;
        
        // e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
        // standalone mode if only one node, else cluster mode.
        char db[128];
        sprintf(db,"%s:%d",(const char*)config.value("dbhost"),(int)config.value("dbport"));
        spdb=std::make_shared<redis_proxy>();
        if(!spdb->connect(db))
            spdb.reset();
    }else{
        Logger<<"server start error: no config file"<<endf;
    }
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
    //Logger<<"on_open\n";
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void Immortal::on_close(svc_handler& sh) {
    auto shid=sh.id();
    players.erase(shid);
    //Logger<<"on_close\n";
}

void Immortal::on_read(svc_handler& sh, void* buf, size_t sz) {
    auto shid=sh.id();
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    if(mid==proto3::pb_msg::MSG_CN_CONNECT){
        MsgNCConnect omsg;
        do{
            MsgCNConnect imsg;
            if(!pb.Parse(imsg)){
                omsg.set_result(proto3::pb_enum::ERR_PROTOCOL);
                break;
            }

            //access player from db
            if(!spdb){
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
                break;
            }

            auto spPlayer=std::make_shared<Player>(sh);
            auto player=spPlayer->playData.mutable_player();
            player->set_uid(imsg.uid());
            
            //fill player
            char key[64];
            sprintf(key,"player:%s",imsg.uid().c_str());
            std::map<std::string,std::string> pmap;
            spdb->hgetall(key,pmap);
            
            if(pmap.count("level")) player->set_level(  atoi(pmap["level"].c_str()));
            if(pmap.count("xp"))    player->set_xp(     atoi(pmap["xp"].c_str()));
            if(pmap.count("gold"))  player->set_gold(   atoi(pmap["gold"].c_str()));
            if(pmap.count("silver"))player->set_silver( atoi(pmap["silver"].c_str()));
            if(pmap.count("energy"))player->set_energy( atoi(pmap["energy"].c_str()));
            
            addPlayer(shid,spPlayer);
            
            omsg.set_result(proto3::pb_enum::SUCCEESS);
            omsg.mutable_player()->CopyFrom(*player);
            //Logger<<"client connected,uid="<<imsg.uid()<<"\n";
        }while (false);
        omsg.set_mid(proto3::pb_msg::MSG_NC_CONNECT);
        PBHelper::Send(sh,omsg);
    }
    else if(mid==proto3::pb_msg::MSG_CL_REPLAY){
        //TODO: move into Lobby
        MsgCLReplay imsg;
        MsgLCReplay omsg;
        if(pb.Parse(imsg)){
            if(!replays.empty())
                omsg.CopyFrom(*replays.front());
        }
        else
        {
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
        }
        omsg.set_mid(proto3::pb_msg::MSG_LC_REPLAY);
        PBHelper::Send(sh,omsg);
    }
    else{
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
