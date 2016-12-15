//
//  Charge.h
//  Charge
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Charge_h
#define Charge_h

class Charge :public Server {
public:
                    Charge();
    virtual void	on_http(const http_parser& req,http_parser& resp,const std::function<void(const http_parser&)> func);
    virtual bool	on_timer(keye::svc_handler&, size_t id, size_t milliseconds);

    MsgHandler      handler;
    int             quantity(float money);
    void            order(const proto3::MsgCPOrder&,proto3::MsgPCOrder&);

    static Charge*   sCharge;
};

#endif /* Charge_h */
