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

inline pb_msg extractBody(std::string& body,const char* inbody);
inline void split_line(std::vector<std::string>& o,std::string& line,char c);

void MsgHandler::on_http(const http_parser& req,http_parser& resp){
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
    
    //decode
    KEYE_LOG("body=%s\n",content.c_str());
    auto str=base64_decode(content);
    KEYE_LOG("decode=%s\n",str.c_str());
    
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
                    KEYE_LOG("client login failed\n");
                    PBHelper::Response(resp,omsg,mid);
                    break;
                }
                
                //account
                auto& account=imsg.user().account();
                auto player=omsg.mutable_player();
                auto spdb=Lobby::sLobby->spdb;
                std::string uid;
                char key[128];
                
                //search db by account
                sprintf(key,"user:%s",account.c_str());
                spdb->hget(key,"uid",uid);
                
                if(uid.empty()){
                    //test udid if not found
                    if(imsg.user().udid()!=account){
                        sprintf(key,"user:%s",imsg.user().udid().c_str());
                        spdb->hget(key,"uid",uid);
                        
                        //bind account if udid exists
                        if(!uid.empty()){
                            spdb->hset(key,"account",account.c_str());
                        }
                    }
                }
                
                if(uid.empty()){
                    //new user
                    char idkey[32];
                    auto rnd=rand()%4;
                    sprintf(idkey,"global_id:%d",rnd+1);
                    spdb->lock(idkey);
                    {
                        spdb->get(idkey,uid);
                        spdb->incrby(idkey);
                    }
                    spdb->unlock(idkey);
                    
                    char tm[32];
                    sprintf(tm,"%ld",time(nullptr));
                    
                    std::map<std::string,std::string> hmap;
                    hmap["uid"]=uid;
                    hmap["regtime"]=tm;
                    hmap["account"]=account;
                    hmap["udid"]=imsg.user().udid();
                    hmap["dev_type"]=imsg.user().dev_type();
                    
                    //new player
                    sprintf(key,"player:%s",uid.c_str());
                    hmap.clear();
                    hmap["level"]="1";
                    hmap["gold"]="10";
                    hmap["silver"]="1000";
                    spdb->hmset(key,hmap);
                    
                    player->set_level(1);
                    player->set_gold(10);
                    player->set_silver(1000);
                }else{
                    std::map<std::string,std::string> hmap;
                    std::vector<std::string> fields;
                    fields.push_back("level");
                    fields.push_back("gold");
                    fields.push_back("silver");
                    if(spdb->hmget(key,fields,hmap)==0){
                        player->set_level(atoi(hmap["level"].c_str()));
                        player->set_gold(atoi(hmap["gold"].c_str()));
                        player->set_silver(atoi(hmap["silver"].c_str()));
                    }
                }

                KEYE_LOG("client login succeeded\n");
                player->set_uid(uid);
                omsg.set_version(imsg.version()+1);
                omsg.set_node("127.0.0.1");
                omsg.set_port(8820);
                omsg.set_result(pb_enum::SUCCEESS);
                
                PBHelper::Response(resp,omsg,mid);
            }else{
                KEYE_LOG("client login failed\n");
                PBHelper::Response(resp,omsg,mid,500,"Internal error");
            }
            break;
        }
        case pb_msg::MSG_CL_LOBBY:{
            MsgCLLobby imsg;
            MsgLCLobby omsg;
            auto omid=pb_msg::MSG_LC_LOBBY;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                KEYE_LOG("client enter succeeded\n");
                omsg.set_result(pb_enum::SUCCEESS);
                
                auto& lobby=*omsg.mutable_lobby();
                lobby.set_version(100);
                lobby.set_bulletin("欢迎进入风云世界！");
                
                auto game=lobby.add_games();
                game->set_id(pb_enum::GAME_PHZ);
                game->set_version(100);
                game->set_desc("风云湖南跑胡子");
                game->add_rules(pb_enum::PHZ_SY);
                game->add_rules(pb_enum::PHZ_SYBP);
                game->add_rules(pb_enum::PHZ_LD);
                
                game->add_rules(pb_enum::PHZ_HH);
                game->add_rules(pb_enum::PHZ_CD_QMT);
                game->add_rules(pb_enum::PHZ_CD_HHD);
                
                game->add_rules(pb_enum::PHZ_CS);
                game->add_rules(pb_enum::PHZ_XX_GHZ);
                game->add_rules(pb_enum::PHZ_HY);
                
                game->add_rules(pb_enum::PHZ_YZ_SBW);
                game->add_rules(pb_enum::PHZ_PEGHZ);
                game->add_rules(pb_enum::PHZ_SC_EQS);
                
                game->add_rules(pb_enum::PHZ_CZ);
                game->add_rules(pb_enum::PHZ_GX);
                
                game=lobby.add_games();
                game->set_id(pb_enum::GAME_MJ);
                game->set_version(100);
                game->set_desc("风云麻将");
                game->add_rules(pb_enum::MJ_SICHUAN);
                game->add_rules(pb_enum::MJ_GUANGDONG);
                game->add_rules(pb_enum::MJ_HUNAN);
                game->add_rules(pb_enum::MJ_FUJIAN);
                game->add_rules(pb_enum::MJ_ZHEJIANG);
                
                game=lobby.add_games();
                game->set_id(pb_enum::GAME_DDZ);
                game->set_version(100);
                game->set_desc("风云斗地主");
                game->add_rules(pb_enum::DDZ_CLASIC);
                game->add_rules(pb_enum::DDZ_FOR4);
            }else{
                KEYE_LOG("client enter failed\n");
                omsg.set_result(pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
            break;
        }
        default:
            break;
    }
}

void split_line(std::vector<std::string>& o,std::string& line,char c){
    std::string comma;
    comma.push_back(c);
    if(!line.empty()&&line.back()!=c)line+=comma;
    while(true){
        auto i=line.find(comma);
        if(i==std::string::npos)break;
        auto str=line.substr(0,i);
        o.push_back(str);
        line=line.substr(++i);
    }
}

pb_msg extractBody(std::string& body,const char* inbody){
    if(inbody){
        std::vector<std::string> params;
        std::string buf(inbody);
        split_line(params,buf,'&');
        
        std::map<std::string,std::string> kvs;
        for(auto& p:params){
            std::vector<std::string> ss;
            split_line(ss,p,'=');
            if(ss.size()>1)
                kvs[ss[0]]=ss[1];
        }
        
        std::string line(inbody);
        
        if(kvs.count("body"))
            body=kvs["body"];
        if(kvs.count("msgid"))
            return (pb_msg)atoi(kvs["msgid"].c_str());
    }
    return pb_msg::MSG_RAW;
}
