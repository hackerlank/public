//
//  PBHelper.h
//  slogin
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef PBHelper_h
#define PBHelper_h

// --------------------------------------------------------
// PBHelper: protobuf helper
// --------------------------------------------------------
class KEYE_API PBHelper{
public:
    static const size_t send_buffer_size=2048;
    PBHelper(keye::PacketWrapper& pw):_pw(pw){
        _packer<<pw;
        _packer>>pw;
    }
    
    bool Parse(google::protobuf::MessageLite& msg){
        auto ret=msg.ParseFromArray(_pw.data,(int)_pw.length);
        if(!ret){
            Logger<<"message parse error: mid="<<(int)Id()<<",length="<<(int)_pw.length<<",buf="<<
            str_util::bytes2hex((const unsigned char*)_pw.data,_pw.length).c_str()<<endl;
        }
        return ret;
    }
    
    proto3::pb_msg Id(){
        proto3::MsgBase mt;
        mt.ParseFromArray(_pw.data,4);
        return (proto3::pb_msg)mt.mid();
    }
    
    static void Send(keye::svc_handler& sh,google::protobuf::MessageLite& msg){
        auto bytes=msg.ByteSize();
        assert(bytes<send_buffer_size);		//large message
        char buffer[send_buffer_size];
        if(msg.SerializeToArray(buffer,bytes)){
            proto3::MsgBase mr;
            if(mr.ParseFromArray(buffer,bytes)){
                assert(mr.mid()>0);
                
                keye::HeadPacker packer;
                keye::PacketWrapper pw(buffer,bytes);
                packer<<pw;
                packer>>pw;
                sh.send(pw.data,pw.length);
                return;
            }
        }
        assert(false);
    }

    static void Request(keye::http_client& http,const char* uri,google::protobuf::MessageLite& msg,proto3::pb_msg mid){
        char smid[8];
        sprintf(smid,"%d",mid);
        std::string str;
        msg.SerializeToString(&str);
        str=base64_encode(str);

        http_parser req(true);
        req.set_uri(uri);
        req.set_method("GET");
        req.set_version("HTTP/1.1");
        req.set_header("msgid",smid);
        req.set_body(str.c_str());
        http.request(req);
    }
    
    static void Response(http_parser& resp,google::protobuf::MessageLite& msg,proto3::pb_msg mid,int code=200,const char* status="OK"){
        char strmid[8];
        sprintf(strmid,"%d",mid);
        std::string str;
        msg.SerializeToString(&str);
        str=base64_encode(str);
        
        resp.set_header("msgid",strmid);
        resp.set_body(str.c_str());
        resp.set_status(code,status);
    }
    //make compiler happy
    void	on_message(keye::svc_handler&,keye::PacketWrapper&){}
private:
    keye::PacketWrapper&    _pw;        //wrapper cached here
    keye::HeadUnpacker      _packer;    //private packer,data cached here
};

#endif /* PBHelper_h */
