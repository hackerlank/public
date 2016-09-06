//
//  GameRule.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"

void GameRule::Tick(){
    KEYE_LOG("----tick desk\n");
    for(auto i:_desks){
        auto& desk=*i.second;
        switch (desk.state) {
            case Desk::ST_WAIT:
                break;
            case Desk::ST_START:
                break;
            case Desk::ST_DISCARD:
                break;
            case Desk::ST_MELD:
                break;
            case Desk::ST_SETTLE:
                break;
            case Desk::ST_END:
                break;
            default:
                break;
        }
    }
}
