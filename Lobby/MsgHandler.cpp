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
        case pb_msg::MSG_CS_LOGIN:{
            MsgCSLogin imsg;
            MsgSCLogin omsg;
            auto mid=pb_msg::MSG_SC_LOGIN;
            omsg.set_mid(mid);
            if(imsg.ParseFromString(str)){
                KEYE_LOG("client login succeeded\n");
                omsg.set_uid("clusters");
                omsg.set_version(imsg.version()+1);
                omsg.set_ip("127.0.0.1");
                omsg.set_port(8810);
                omsg.set_result(pb_enum::SUCCEESS);
                
                PBHelper::Response(resp,omsg,mid);
            }else{
                KEYE_LOG("client login failed\n");
                PBHelper::Response(resp,omsg,mid,500,"Internal error");
            }
            break;
        }
        case pb_msg::MSG_CL_ENTER:{
            auto str=base64_decode(body);
            MsgCLEnter imsg;
            MsgLCEnter omsg;
            auto omid=pb_msg::MSG_LC_ENTER;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                KEYE_LOG("client enter succeeded\n");
                omsg.set_result(pb_enum::SUCCEESS);
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
