//
//  Charge.h
//  Charge
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Charge_h
#define Charge_h

class Charge :public keye::ws_service {
public:
                    Charge(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    void            run(const char* =nullptr);
    virtual void	on_http(const http_parser& req,http_parser& resp);
    virtual bool	on_timer(keye::svc_handler&, size_t id, size_t milliseconds);

    MsgHandler      handler;
    void            setup_log(const char*);
    int             quantity(float money);
    void            order(const proto3::MsgCPOrder&,proto3::MsgPCOrder&);

    static Charge*   sCharge;
    std::shared_ptr<vic_proxy>      spdb;
    std::map<unsigned long,long>    sessions; //[session,timestamp]
};

#endif /* Charge_h */
