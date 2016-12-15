//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "Fwd.h"
using namespace proto3;
using namespace std;

inline pb_msg extractBody(std::string& body,const char* inbody);
inline void split_line(std::vector<std::string>& o,std::string& line,char c);

inline unsigned long genSession(){
    auto tt=keye::ticker();
    srand((int)tt);
    auto r=((unsigned long)rand())<<8;
    auto a=(tt>>0);
    auto b=(tt>>16);
    auto c=(tt>>32);
    auto d=(tt>>48);
    return r + (a<<48) + (d<<32) + (b<<16) + c;
}

MsgHandler::MsgHandler(){
    paySvc.push_back(std::make_shared<AliPaySvc>());
}

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
    
    if(msgid<=pb_msg::MSG_CP_BEGIN || msgid>=pb_msg::MSG_CP_END){
        for(auto pay:paySvc){
            if(pay->on_http(req))
                return;
        }
        Debug<<"invalid message id "<<(int)msgid<<endl;
        return;
    }

    //decode
    Debug<<"body="<<content.c_str()<<endl;
    auto str=base64_decode(content);
    Debug<<"decode="<<str.c_str()<<endl;
    
    auto spdb=Charge::sCharge->spdb;
    //process
    switch(msgid){
        case pb_msg::MSG_CP_LOGIN:{
            //need retrieve uid and store in client first
            proto3::MsgCPLogin imsg;
            proto3::MsgPCLogin omsg;
            auto mid=pb_msg::MSG_PC_LOGIN;
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
                auto& account=imsg.user().account();
                auto md5a=MD5::HashAnsiString(account.c_str());
                auto player=omsg.mutable_player();
                std::string uid;
                char key[128];

                //search db by account
                sprintf(key,"user:%s",md5a.c_str());
                spdb->hget(key,"uid",uid);

                if(uid.empty()){
                    //test udid if not found
                    if(imsg.user().udid()!=account){
                        auto md5udid=MD5::HashAnsiString(imsg.user().udid().c_str());
                        sprintf(key,"user:%s",md5udid.c_str());
                        spdb->hget(key,"uid",uid);

                        //bind account if udid exists
                        if(!uid.empty()){
                            spdb->hset(key,"account",md5a.c_str());
                        }
                    }
                }

                char timestamp[32];
                auto tt=time(nullptr);
                sprintf(timestamp,"%ld",tt);
                if(uid.empty()){
                    //new user
                    Debug<<"client not exists\n";
                    omsg.set_result(pb_enum::ERR_NOTEXISTS);
                }else{
                    std::map<std::string,std::string> hmap;
                    std::vector<std::string> fields;
                    fields.push_back("level");
                    fields.push_back("gold");
                    fields.push_back("silver");
                    sprintf(key,"player:%s",uid.c_str());
                    if(spdb->hmget(key,fields,hmap)!=-1){
                        player->set_level(atoi(hmap["level"].c_str()));
                        player->set_gold(atoi(hmap["gold"].c_str()));
                        player->set_silver(atoi(hmap["silver"].c_str()));
                    }
                    Debug<<"client "<<uid.c_str()<<" login succeeded\n";
                    player->set_uid(uid);
                    auto session=genSession();
                    omsg.set_version(imsg.version()+1);
                    omsg.set_session(session);
                    omsg.set_result(pb_enum::SUCCEESS);
                    
                    Charge::sCharge->sessions[session]=tt;
                }

                PBHelper::Response(func,omsg,mid);
            }else{
                Debug<<"client login failed\n";
                PBHelper::Response(func,omsg,mid,500,"Internal error");
            }
            break;
        }
        case pb_msg::MSG_CP_ORDER:{
            proto3::MsgCPOrder imsg;
            proto3::MsgPCOrder omsg;
            auto mid=pb_msg::MSG_PC_ORDER;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                //account
                auto& uid=imsg.uid();
                if(Charge::sCharge->sessions.count(imsg.session())){
                    //not found
                    Debug<<"client "<<uid.c_str()<<" not exists\n";
                    omsg.set_result(pb_enum::ERR_NOTEXISTS);
                }else{
                    Charge::sCharge->order(imsg,omsg);
                    omsg.set_result(pb_enum::SUCCEESS);
                    Debug<<"client "<<uid.c_str()<<" ordered "<<
                        (int)imsg.amount()<<","<<omsg.appscheme().c_str()<<","<<omsg.orderstring().c_str()<<"\n";
                }
                
                PBHelper::Response(func,omsg,mid);
            }else{
                Debug<<"client order failed\n";
                PBHelper::Response(func,omsg,mid,500,"Internal error");
            }
            break;
        }
        case pb_msg::MSG_CP_VERIFY:{
            proto3::MsgCPVerify imsg;
            proto3::MsgPCVerify omsg;
            auto mid=pb_msg::MSG_PC_VERIFY;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                //account
                auto& uid=imsg.uid();
                if(Charge::sCharge->sessions.count(imsg.session())){
                    //not found
                    Debug<<"client "<<uid.c_str()<<" not exists\n";
                    omsg.set_result(pb_enum::ERR_NOTEXISTS);
                }else{
                    auto gold=Charge::sCharge->quantity(imsg.total_amount());
                    
                    char key[128];
                    sprintf(key,"player:%s",uid.c_str());
                    if(spdb->hincrby(key,"gold",gold)){
                        auto player=omsg.mutable_player();
                        player->set_uid(uid);
                        player->set_gold(gold);
                        omsg.set_result(pb_enum::SUCCEESS);
                        Logger<<"client "<<uid.c_str()<<" charged "<<(int)imsg.total_amount()<<" for "<<gold<<"\n";
                    }
                }
                
                PBHelper::Response(func,omsg,mid);
            }else{
                Debug<<"client order failed\n";
                PBHelper::Response(func,omsg,mid,500,"Internal error");
            }
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
