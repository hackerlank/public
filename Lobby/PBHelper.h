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
class PBHelper{
public:
    static const size_t send_buffer_size=2048;
    PBHelper(keye::PacketWrapper& pw):_pw(pw){
        keye::HeadUnpacker packer;
        packer<<pw;
        packer>>pw;
    }
    
    bool Parse(google::protobuf::MessageLite& msg){
        return msg.ParseFromArray(_pw.data,(int)_pw.length);
    }
    
    eMsg Id(){
        proto3::MsgBase mt;
        mt.ParseFromArray(_pw.data,4);
        return (eMsg)mt.mid();
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
    //make compiler happy
    void	on_message(keye::svc_handler&,keye::PacketWrapper&){}
private:
    keye::PacketWrapper& _pw;
};

#endif /* PBHelper_h */
