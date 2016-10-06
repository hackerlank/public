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

#include "PBHelper.h"

typedef short unit_id_t;
typedef short pos_t;
typedef short color_t;
typedef short value_t;
typedef unsigned game_id_t;

const unit_id_t i_invalid=(unit_id_t)-1;

#include "Player.h"
#include "Game.h"
#include "GameRule.h"
#include "GameRule/DoudeZhu.h"
#include "GameRule/Mahjong.h"
#include "Node.h"

#endif /* login_fwd_h */
