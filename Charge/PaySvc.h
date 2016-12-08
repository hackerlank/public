//
//  PaySvc.h
//  PaySvc
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef PaySvc_h
#define PaySvc_h

class PaySvc {
public:
    virtual bool	on_http(const http_parser& req,http_parser& resp)=0;
};

class AliPaySvc:public PaySvc{
public:
    virtual bool	on_http(const http_parser& req,http_parser& resp);
};

#endif /* PaySvc_h */
