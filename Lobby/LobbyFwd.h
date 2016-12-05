//
//  robot_fwd.h
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef login_fwd_h
#define login_fwd_h

#include <keye/htio/htio_demo.h>

#include "protocol.pb.h"

extern std::shared_ptr<keye::logger> sLogger;
#ifndef Logger
#define Logger sLogger->operator<<(begl)
#endif

#include "PBHelper.h"
#include "MsgHandler.h"
#include "Lobby.h"

#endif /* login_fwd_h */
