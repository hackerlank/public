// test_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NodeFwd.h"

#ifdef WIN32
#include <conio.h>
#endif

using namespace keye;
using namespace proto3;

enum TIMER:size_t{
    TIMER_SEC=100,
    TIMER_MIN,
    TIMER_HOUR,
    TIMER_DAY,
};

Node* Node::sNode=nullptr;

Node::Node(size_t ios, size_t works, size_t rb_size)
:ws_service(ios, works, rb_size)
,_game_index(0){
    sNode=this;
    
    registerRule(std::make_shared<DoudeZhu>());
    registerRule(std::make_shared<Mahjong>());
}

void Node::registerRule(std::shared_ptr<GameRule> game){
    auto id=game->Type();
    gameRules[id]=game;
}

std::shared_ptr<Game> Node::createGame(int key,proto3::MsgCNCreate& msg){
    std::shared_ptr<Game> gameptr;
    auto rule=gameRules.find(msg.game());
    if(rule!=gameRules.end()&&key>0){
        auto gid=key*pb_enum::DEF_MAX_GAMES_PER_NODE+_game_index++;
        gameptr=std::make_shared<Game>();
        games[gid]=gameptr;
        gameptr->id=gid;
        gameptr->rule=rule->second;
    }else
        KEYE_LOG("create game error no rule %d\n",msg.game());
    return gameptr;
}

std::shared_ptr<Game> Node::findGame(game_id_t id){
    return games.count(id)>0?games[id]:std::shared_ptr<Game>();
}

void Node::removeGame(game_id_t id){
    gameRules.erase(id);
}

void Node::on_open(svc_handler&) {
//    KEYE_LOG("on_open\n");
    //set_timer(WRITE_TIMER, WRITE_FREQ);
}

void Node::on_close(svc_handler& sh) {
    auto shid=sh.id();
    players.erase(shid);
//    KEYE_LOG("on_close\n");
}

void Node::on_read(svc_handler& sh, void* buf, size_t sz) {
    auto shid=sh.id();
    keye::PacketWrapper pw(buf,sz);
    PBHelper pb(pw);
    auto mid=pb.Id();
    if(mid==proto3::pb_msg::MSG_CN_ENTER){
        MsgCNEnter imsg;
        MsgNCEnter omsg;
        if(pb.Parse(imsg)){
            omsg.set_result(proto3::pb_enum::SUCCEESS);
            auto game=omsg.mutable_player();
            game->set_level(168);
            game->set_uid("vic-game");
            game->set_currency(1000);
            KEYE_LOG("client entered\n");
            
            players[shid]=std::make_shared<Player>(sh);
        }else{
            KEYE_LOG("message error id=%zd\n",mid);
            omsg.set_result(proto3::pb_enum::ERR_FAILED);
        }
        omsg.set_mid(proto3::pb_msg::MSG_NC_ENTER);
        PBHelper::Send(sh,omsg);
    }else{
        auto p=players.find(shid);
        if(p!=players.end())
            p->second->on_read(pb);
    }
    //KEYE_LOG("on_read %zd,mid=%d\n", sz,mid);
}

void Node::on_write(svc_handler&, void*, size_t sz) {
//    KEYE_LOG("on_write %zd\n",sz);
}

bool Node::on_timer(svc_handler&, size_t id, size_t milliseconds) {
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

int main(int argc, char* argv[]) {
    unsigned short port = 8820;
    for(auto i=1;i<argc;++i){
        auto arg=argv[i];
        if(strlen(arg)>3&&arg[0]=='-')switch(arg[1]){
            case 'p':{
                auto a=&arg[2];
                try{
                    port=atoi(a);
                }catch(...){}
                break;
            }
            case 'w':
                break;
            case 'i':
            default:
                break;
        }
    }
	proto3::MsgSCLogin zi;

	redis_proxy redis;
	keye::PacketWrapper pw;
	PBHelper helper(pw);

    Node server;
	server.run(port,"127.0.0.1");
    server.set_timer(TIMER::TIMER_SEC, 1000);
    server.set_timer(TIMER::TIMER_MIN, 1000*60);
    server.set_timer(TIMER::TIMER_HOUR,1000*60*60);
    server.set_timer(TIMER::TIMER_DAY, 1000*60*60*24);

    //DoudeZhu::test();
    
    KEYE_LOG("++++server start at %d\n", port);
	std::getchar();

	return 0;
}
