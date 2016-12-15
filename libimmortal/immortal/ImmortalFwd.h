//
//  robot_fwd.h
//  robot
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef immortal_fwd_h
#define immortal_fwd_h

#include <keye/htio/htio_demo.h>
#include <algorithm>

#include <protocol.pb.h>

typedef short unit_id_t;
typedef short pos_t;
typedef short color_t;
typedef short value_t;
typedef unsigned game_id_t;
static const unit_id_t invalid_card=-1; //more reliable than 0

extern std::shared_ptr<keye::logger> sLogger;
extern std::shared_ptr<keye::logger> sDebug;
#ifndef Logger
#define Logger sLogger->operator<<(begl)
#endif
#ifndef Debug
#define Debug sDebug->operator<<(begl)
#endif

#include "PBHelper.h"
#include "Server.h"
#include <immortal/Player.h>
#include <immortal/Game.h>
#include <immortal/GameRule.h>
#include <immortal/DiscardGame.h>
#include <immortal/MeldGame.h>
#include <immortal/Immortal.h>

#endif /* immortal_fwd_h */
