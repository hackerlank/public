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
                string strUdid(imsg.user().udid());
                Charge::sCharge->tpool.schedule(std::bind([](
                                                             const string Account,
                                                             const string Udid,
                                                             const std::function<void(const http_parser&)> Func){
                    auto Spdb=Charge::sCharge->spdb;
                    proto3::MsgPCLogin omsg;
                    auto mid=pb_msg::MSG_PC_LOGIN;
                    omsg.set_mid(mid);
                    
                    //search db by account
                    char key[128];
                    auto md5a=MD5::HashAnsiString(Account.c_str());
                    sprintf(key,"user:%s",md5a.c_str());
                    std::string uid;
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
                    
                    if(uid.empty()){
                        //new user
                        Debug<<"client not exists\n";
                        omsg.set_result(pb_enum::ERR_NOTEXISTS);
                    }else{
                        char timestamp[32];
                        auto tt=time(nullptr);
                        sprintf(timestamp,"%ld",tt);
                        auto player=omsg.mutable_player();
                        std::map<std::string,std::string> hmap;
                        std::vector<std::string> fields;
                        fields.push_back("level");
                        fields.push_back("gold");
                        fields.push_back("silver");
                        sprintf(key,"player:%s",uid.c_str());
                        if(Spdb->hmget(key,fields,hmap)!=-1){
                            player->set_level(atoi(hmap["level"].c_str()));
                            player->set_gold(atoi(hmap["gold"].c_str()));
                            player->set_silver(atoi(hmap["silver"].c_str()));
                        }
                        Debug<<"client "<<uid.c_str()<<" login succeeded\n";
                        auto version = (int)Charge::sCharge->config.value("version");
                        player->set_uid(uid);
                        auto session=genSession();
                        omsg.set_version(version);
                        omsg.set_session(session);
                        omsg.set_result(pb_enum::SUCCEESS);
                        
                        Charge::sCharge->sessions[session]=tt;
                    }
                    
                    PBHelper::Response(Func,omsg,mid);
                },account,strUdid,func));
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
                if(!Charge::sCharge->sessions.count(imsg.session())){
                    //not found
                    Debug<<"client "<<uid.c_str()<<" invalid session\n";
                    omsg.set_result(pb_enum::ERR_SESSION);
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
                if(!Charge::sCharge->sessions.count(imsg.session())){
                    //not found
                    Debug<<"client "<<uid.c_str()<<" invalid session\n";
                    omsg.set_result(pb_enum::ERR_SESSION);
                    
                    PBHelper::Response(func,omsg,mid);
                }else{
                    Charge::sCharge->tpool.schedule(std::bind([](
                                                                 const int Amount,
                                                                 const string Uid,
                                                                 const std::function<void(const http_parser&)> Func){
                        auto Spdb=Charge::sCharge->spdb;
                        proto3::MsgPCVerify omsg;
                        auto mid=pb_msg::MSG_PC_VERIFY;
                        omsg.set_mid(mid);

                        char key[128];
                        sprintf(key,"player:%s",Uid.c_str());
                        auto gold=Charge::sCharge->quantity(Amount);
                        if(Spdb->hincrby(key,"gold",gold)){
                            auto player=omsg.mutable_player();
                            player->set_uid(Uid);
                            player->set_gold(gold);
                            omsg.set_result(pb_enum::SUCCEESS);
                            Logger<<"client "<<Uid.c_str()<<" charged "<<(int)Amount<<" for "<<gold<<"\n";
                        }else
                            omsg.set_result(pb_enum::ERR_DB);
                        
                        PBHelper::Response(Func,omsg,mid);
                    },imsg.total_amount(),uid,func));
                }
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
