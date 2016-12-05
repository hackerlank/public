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

void MsgHandler::on_http(const http_parser& req,http_parser& resp){
    auto msgid=req.header("msgid");
    auto mid=(pb_msg)atoi(msgid);
    if(mid<pb_msg::MSG_CH_BEGIN||mid>pb_msg::MSG_CH_END){
        Logger<<"invalid msg id "<<(int)mid<<endl;
        return;
    }
    
    if(mid<=pb_msg::MSG_CH_BEGIN || mid>=pb_msg::MSG_CH_END){
        Logger<<"invalid message id "<<(int)mid<<endl;
        return;
    }

    auto body=req.body();
    auto str=base64_decode(body);
    switch(mid){
        case pb_msg::MSG_CH_LOGIN:{
            proto3::MsgCHLogin imsg;
            proto3::MsgHCLogin omsg;
            auto omid=pb_msg::MSG_HC_LOGIN;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                //query uid from db by phone
                //query amount from db by uid
                //generate session for charger
                omsg.set_amount(1999);
                omsg.set_session(19700101);
                
                Logger<<"login succeeded\n";
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                Logger<<"login failed\n";
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
            break;
        }
        case pb_msg::MSG_CH_REGISTER:{
            proto3::MsgCHRegister imsg;
            proto3::MsgHCRegister omsg;
            auto omid=pb_msg::MSG_HC_REGISTER;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                //query db by phone and uid
                //insert into db if not exists
                
                Logger<<"register succeeded\n";
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                Logger<<"register failed\n";
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
            break;
        }
        case pb_msg::MSG_CH_CHARGE:{
            proto3::MsgCHCharge imsg;
            proto3::MsgHCCharge omsg;
            auto omid=pb_msg::MSG_HC_CHARGE;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                //verify session and retrieve phone
                //query uid and amount from db by phone
                //transfer from source uid to target
                
                Logger<<"charge succeeded\n";
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                Logger<<"charge failed\n";
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
            break;
        }
        case pb_msg::MSG_CH_QUERY:{
            proto3::MsgCHQuery imsg;
            proto3::MsgHCQuery omsg;
            auto omid=pb_msg::MSG_HC_QUERY;
            omsg.set_mid(omid);
            if(imsg.ParseFromString(str)){
                //verify session and retrieve phone
                //query charge logs from db by target uid
                
                Logger<<"query succeeded\n";
                omsg.set_result(proto3::pb_enum::SUCCEESS);
            }else{
                Logger<<"query failed\n";
                omsg.set_result(proto3::pb_enum::ERR_FAILED);
            }
            PBHelper::Response(resp,omsg,omid);
            break;
        }
        default:
            break;
    }
}
