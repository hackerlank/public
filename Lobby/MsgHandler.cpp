//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "LobbyFwd.h"
using namespace proto3;
using namespace std;

void MsgHandler::on_http(const http_parser& req,const std::function<void(const http_parser&)> func){
    auto strmid=req.header("msgid");
    auto body=req.body();
    
    //extra msgid and content
    std::string content;
    auto msgid=pb_msg::MSG_RAW;
    if(strmid&&strlen(strmid)>0){
        msgid=(pb_msg)atoi(strmid);
        content=body;   //TODO:optimize
    }else
        msgid=extractBody(content,body);
    
    if(msgid<=pb_msg::MSG_CL_BEGIN || msgid>=pb_msg::MSG_CL_END){
        Debug<<"invalid message id "<<(int)msgid<<endl;
        return;
    }
    
    //decode
    //Debug<<"body="<<content.c_str()<<endl;
    auto str=base64_decode(content);
    //Debug<<"decode="<<str.c_str()<<endl;
    
    //process
    switch(msgid){
        case pb_msg::MSG_CL_LOGIN:{
            MsgCLLogin imsg;
            MsgLCLogin omsg;
            auto mid=pb_msg::MSG_LC_LOGIN;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                //version
                if(imsg.version()<100){
                    omsg.set_result(pb_enum::ERR_VERSION);
                    Debug<<"client login failed\n";
                    PBHelper::Response(func,omsg,mid);
                    break;
                }
                
                //account
                Lobby::sLobby->tpool.schedule(std::bind([](
                                                             const string Account,
                                                             const string Udid,
                                                             pb_enum dev_type,
                                                             const std::function<void(const http_parser&)> Func){

                    MsgLCLogin omsg;
                    auto mid=pb_msg::MSG_LC_LOGIN;
                    omsg.set_mid(mid);
                    
                    auto md5a=MD5::HashAnsiString(Account.c_str());
                    auto player=omsg.mutable_player();
                    std::string uid;
                    char key[128];
                    
                    //search db by account
                    auto Spdb=Lobby::sLobby->spdb;
                    sprintf(key,"user:%s",md5a.c_str());
                    Spdb->hget(key,"uid",uid);
                    
                    if(uid.empty()){
                        //test udid if not found
                        if(Udid!=Account){
                            auto md5udid=MD5::HashAnsiString(Udid.c_str());
                            sprintf(key,"user:%s",md5udid.c_str());
                            Spdb->hget(key,"uid",uid);
                            
                            //bind account if udid exists
                            if(!uid.empty()){
                                Spdb->hset(key,"account",md5a.c_str());
                            }
                        }
                    }
                    
                    char timestamp[32];
                    auto tt=time(nullptr);
                    sprintf(timestamp,"%ld",tt);
                    if(uid.empty()){
                        //new user
                        char idkey[32];
                        auto rnd=rand()%4;
                        sprintf(idkey,"global_id:%d",rnd+1);
                        Spdb->lock(idkey);
                        {
                            Spdb->get(idkey,uid);
                            Spdb->incrby(idkey);
                        }
                        Spdb->unlock(idkey);
                        
                        std::map<std::string,std::string> hmap;
                        hmap["uid"]=uid;
                        hmap["regtime"]=timestamp;
                        hmap["lastlogin"]=timestamp;
                        hmap["account"]=md5a;
                        hmap["udid"]=Udid;
                        hmap["dev_type"]=dev_type;
                        Spdb->hmset(key,hmap);
                        
                        //new player
                        string defaultgold,defaultsilver;
                        str_util::wstr2str(defaultgold,(const wchar_t*)Lobby::sLobby->config.value(L"defaultgold"));
                        str_util::wstr2str(defaultsilver,(const wchar_t*)Lobby::sLobby->config.value(L"defaultsilver"));
                        sprintf(key,"player:%s",uid.c_str());
                        hmap.clear();
                        hmap["level"]="1";
                        hmap["gold"]=defaultgold;
                        hmap["silver"]=defaultsilver;
                        Spdb->hmset(key,hmap);
                        
                        player->set_level(1);
                        player->set_gold(atoi(defaultgold.c_str()));
                        player->set_silver(atoi(defaultsilver.c_str()));
                    }else{
                        Spdb->hset(key,"lastlogin",timestamp);
                        
                        sprintf(key,"player:%s",uid.c_str());
                        std::map<std::string,std::string> hmap;
                        std::vector<std::string> fields;
                        fields.push_back("level");
                        fields.push_back("gold");
                        fields.push_back("silver");
                        if(Spdb->hmget(key,fields,hmap)!=-1){
                            player->set_level(atoi(hmap["level"].c_str()));
                            player->set_gold(atoi(hmap["gold"].c_str()));
                            player->set_silver(atoi(hmap["silver"].c_str()));
                        }
                    }//uid.empty()
                    
                    Debug<<"client "<<uid.c_str()<<" login\n";
                    auto version = (int)Lobby::sLobby->config.value(L"version");
                    auto session=genSession();
                    omsg.set_version(version);
                    omsg.set_session(session);
                    player->set_uid(uid);
                    omsg.set_result(pb_enum::SUCCEESS);
                    
                    Lobby::sLobby->sessions[session]=tt;

                    PBHelper::Response(Func,omsg,mid);
                },imsg.user().account(), imsg.user().udid(), imsg.user().dev_type(), func));

            }else{
                Debug<<"client login failed\n";
                PBHelper::Response(func,omsg,mid,500,"Internal error");
            }
            break;
        }
        case pb_msg::MSG_CL_LOBBY:{
            MsgCLLobby imsg;
            MsgLCLobby omsg;
            auto omid=pb_msg::MSG_LC_LOBBY;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                if(Lobby::sLobby->sessions.count(imsg.session())){
                    omsg.set_result(pb_enum::SUCCEESS);
                    
                    auto& lobby=*omsg.mutable_lobby();
                    lobby.set_version(100);
                    lobby.set_bulletin("欢迎进入风云世界！");
                    lobby.mutable_games()->CopyFrom(Lobby::sLobby->gameConfig);
                }else{
                    //not found
                    Debug<<"client "<<imsg.uid().c_str()<<" invalid session\n";
                    omsg.set_result(pb_enum::ERR_SESSION);
                }
            }else{
                Debug<<"client enter failed\n";
                omsg.set_result(pb_enum::ERR_PROTOCOL);
            }
            PBHelper::Response(func,omsg,omid);
            break;
        }
        case proto3::pb_msg::MSG_CL_REPLAYS:{
            MsgCLReplays imsg;
            const auto mid=pb_msg::MSG_LC_REPLAYS;
            MsgLCReplays omsg;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                if(Lobby::sLobby->sessions.count(imsg.session())){
                    Lobby::sLobby->tpool.schedule(std::bind([](
                                                               const string Uid,
                                                               const std::function<void(const http_parser&)> Func){
                        MsgLCReplays omsg;
                        omsg.set_mid(mid);
                        
                        //player replay list - replay:player:<uid>{game id list}
                        auto Spdb=Lobby::sLobby->spdb;
                        char key[32];
                        sprintf(key,"replay:player:%s",Uid.c_str());
                        std::vector<std::string> values;
                        Spdb->lrange(key,0,-1,values);
                        for(auto& v:values){
                            replays game_replay;
                            if(game_replay.ParseFromString(v))
                                omsg.add_all()->CopyFrom(game_replay);
                        }
                        omsg.set_result(proto3::pb_enum::SUCCEESS);
                        PBHelper::Response(Func,omsg,mid);
                    },imsg.uid(),func));
                    break;
                }
                
                //not found
                Debug<<"client "<<imsg.uid().c_str()<<" invalid session\n";
                omsg.set_result(pb_enum::ERR_SESSION);
            }
            else
            {
                omsg.set_result(proto3::pb_enum::ERR_PROTOCOL);
            }
            PBHelper::Response(func,omsg,mid);
            break;
        }
        case proto3::pb_msg::MSG_CL_REPLAY:{
            MsgCLReplay imsg;
            const auto mid=pb_msg::MSG_LC_REPLAY;
            MsgLCReplay omsg;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                if(Lobby::sLobby->sessions.count(imsg.session())){
                    Lobby::sLobby->tpool.schedule(std::bind([](
                                                               const int gameid,
                                                               const int round,
                                                               const std::function<void(const http_parser&)> Func){
                        
                        MsgLCReplay omsg;
                        omsg.set_mid(mid);
                        
                        //replay hash data - replay:<game id>{round:data}
                        auto Spdb=Lobby::sLobby->spdb;
                        char key[32],field[32];
                        sprintf(key,"replay:%d",gameid);
                        sprintf(field,"%d",round);
                        std::string buf;
                        Spdb->hget(key,field,buf);
                        
                        if(omsg.mutable_data()->ParseFromString(buf)){
                            omsg.set_result(pb_enum::SUCCEESS);
                        }else{
                            omsg.set_result(pb_enum::ERR_FAILED);
                            omsg.clear_data();
                        }
                        
                        PBHelper::Response(Func,omsg,mid);
                    },imsg.gameid(),imsg.round(),func));
                    break;
                }
                
                //not found
                Debug<<"client invalid session\n";
                omsg.set_result(pb_enum::ERR_SESSION);
            }
            else
            {
                omsg.set_result(pb_enum::ERR_PROTOCOL);
            }
            PBHelper::Response(func,omsg,mid);
            break;
        }
        default:
            break;
    }
}
