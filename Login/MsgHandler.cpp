//
//  MsgHandler.cpp
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "LoginFwd.h"
using namespace proto3;

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

proto3::pb_msg extractBody(std::string& body,const char* inbody){
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
            return (proto3::pb_msg)atoi(kvs["msgid"].c_str());
    }
    return proto3::pb_msg::MSG_RAW;
}

void MsgHandler::on_http(const http_parser& req,http_parser& resp){
    auto strmid=req.header("msgid");
    auto body=req.body();
    
    std::string content;
    auto msgid=proto3::pb_msg::MSG_RAW;
    if(strmid&&strlen(strmid)>0)
        msgid=(proto3::pb_msg)atoi(strmid);
    else
        msgid=extractBody(content,body);
    
    if(msgid==proto3::pb_msg::MSG_CS_LOGIN){
        KEYE_LOG("body=%s\n",content.c_str());
        auto str=base64_decode(content);
        KEYE_LOG("decode=%s\n",str.c_str());
        proto3::MsgCSLogin imsg;
        proto3::MsgSCLogin omsg;
        auto mid=proto3::pb_msg::MSG_SC_LOGIN;
        omsg.set_mid(mid);
        if(imsg.ParseFromString(str)){
            KEYE_LOG("client login succeeded\n");
            omsg.set_uid("clusters");
            omsg.set_version(imsg.version()+1);
            omsg.set_ip("127.0.0.1");
            omsg.set_port(8810);
            omsg.set_result(proto3::pb_enum::SUCCEESS);
            
            PBHelper::Response(resp,omsg,mid);
        }else{
            KEYE_LOG("client login failed\n");
            PBHelper::Response(resp,omsg,mid,500,"Internal error");
        }
    }
}
