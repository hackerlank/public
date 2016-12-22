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

Immortal* Immortal::sImmortal=nullptr;

Immortal::Immortal()
:Server("node")
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
        Debug<<"create game error no rule "<<msg.game()<<endl;
    return gameptr;
}

std::shared_ptr<Game> Immortal::findGame(game_id_t id){
    return games.count(id)>0?games[id]:std::shared_ptr<Game>();
}

void Immortal::removeGame(game_id_t id){
    games.erase(id);
}

void Immortal::addPlayer(size_t shid,std::shared_ptr<Player> sp){
    if(!sp)
        return;
    players[shid]=sp;;
}

void Immortal::on_open(svc_handler&) {
    //Debug<<"on_open\n";
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void Immortal::on_close(svc_handler& sh) {
    auto shid=sh.id();
    players.erase(shid);
    //Debug<<"on_close\n";
}

void Immortal::on_read(svc_handler& sh, void* buf, size_t sz) {
    auto shid=sh.id();
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    
    if(mid<=pb_msg::MSG_CN_BEGIN || mid>=pb_msg::MSG_CN_END){
        Debug<<"invalid message id "<<(int)mid<<endl;
        return;
    }

    if(mid==proto3::pb_msg::MSG_CN_CONNECT){
        MsgNCConnect omsg;
        const auto omid=proto3::pb_msg::MSG_NC_CONNECT;
        do{
            MsgCNConnect imsg;
            if(!pb.Parse(imsg)){
                omsg.set_result(proto3::pb_enum::ERR_PROTOCOL);
                break;
            }

            if(!spdb){
                omsg.set_result(proto3::pb_enum::ERR_DB);
                break;
            }

            //access player from db
            auto spPlayer=std::make_shared<Player>(sh);
            Immortal::sImmortal->tpool.schedule(std::bind([](
                                                             std::shared_ptr<svc_handler> Spsh,
                                                             std::string Uid,
                                                             decltype(spPlayer) SpPlayer){

                MsgNCConnect omsg;
                auto player=SpPlayer->playData.mutable_player();
                player->set_uid(Uid);
                auto Shid=Spsh->id();
                
                //fill player
                char key[64];
                sprintf(key,"player:%s",Uid.c_str());
                std::map<std::string,std::string> pmap;
                Immortal::sImmortal->spdb->hgetall(key,pmap);
                
                if(pmap.count("level")) player->set_level(  atoi(pmap["level"].c_str()));
                if(pmap.count("xp"))    player->set_xp(     atoi(pmap["xp"].c_str()));
                if(pmap.count("gold"))  player->set_gold(   atoi(pmap["gold"].c_str()));
                if(pmap.count("silver"))player->set_silver( atoi(pmap["silver"].c_str()));
                if(pmap.count("energy"))player->set_energy( atoi(pmap["energy"].c_str()));
                
                Immortal::sImmortal->addPlayer(Shid,SpPlayer);
                
                auto version = (int)Immortal::sImmortal->config.value(L"version");
                omsg.set_result(proto3::pb_enum::SUCCEESS);
                omsg.mutable_player()->CopyFrom(*player);
                omsg.set_mid(omid);
                omsg.set_version(version);
                PBHelper::Send(*Spsh,omsg);
                //Debug<<"client connected,uid="<<imsg.uid()<<"\n";
            },sh(), imsg.uid(), spPlayer));
            
            return;

        }while (false);
        omsg.set_mid(omid);
        PBHelper::Send(sh,omsg);
    }
    else{
        auto p=players.find(shid);
        if(p!=players.end())
            p->second->on_read(pb);
    }
    //Debug<<"on_read %zd,mid=%d\n", sz,mid);
}

void Immortal::on_write(svc_handler&, void*, size_t sz) {
//    Debug<<"on_write %zd\n",sz);
}

bool Immortal::on_timer(svc_handler& sh, size_t id, size_t milliseconds) {
    switch (id) {
        case TIMER::TIMER_SEC:
            //tick game
            for(auto iter=games.begin();iter!=games.end();){
                auto game=iter->second;
                game->rule->Tick(*game);
                if(game->state==Game::ST_END){
                    game->rule->Release(*game);
                    iter=games.erase(iter);
                }else
                    ++iter;
            }
            break;
            
        case TIMER::TIMER_MIN:{
            //dismiss game
            auto ts=time(nullptr);
            auto lifetime=decltype(ts)(60*60*8);  //8 hours
            auto dismisstime=decltype(ts)(60*5);  //5 minutes
            auto lt=(int)config.value("game_life_time");
            auto dt=(int)config.value("dismiss_time");
            if(lt>0)lifetime=lt;
            if(dt>0)dismisstime=dt;
            for(auto iter=games.begin();iter!=games.end();++iter){
                auto game=iter->second;
                if(ts - game->start_timestamp >= lifetime
                    || ts - game->dismiss_timestamp >= dismisstime)
                    game->rule->Dismiss(*game);
            }
            break;
        }
        case TIMER::TIMER_HOUR:{
            //setup log every day in the morning
            time_t t=time(NULL);
            tm* aTm=localtime(&t);
            if(aTm->tm_hour==0)
                setup_log(name.c_str());
            break;
        }
        case TIMER::TIMER_DAY:
            break;
        default:
            break;
    }
    return Server::on_timer(sh,id,milliseconds);
}
