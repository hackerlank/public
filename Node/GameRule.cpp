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
    for(auto i:_desks){
        auto& desk=*i.second;
        switch (desk.state) {
            case Desk::State::ST_WAIT:
                if(Ready(desk))
                    ChangeState(desk,Desk::State::ST_START);
                break;
            case Desk::State::ST_START:
                Deal(desk);
                ChangeState(desk,Desk::State::ST_DISCARD);
                break;
            case Desk::State::ST_DISCARD:
                break;
            case Desk::State::ST_MELD:
                break;
            case Desk::State::ST_SETTLE:
                break;
            case Desk::State::ST_END:
                break;
            default:
                break;
        }
    }
}

Desk* GameRule::Create(Player& user,std::shared_ptr<Desk> deskptr){
    if(deskptr){
        ++deskptr->ready;
        _desks[deskptr->id]=deskptr;
    }
    return deskptr.get();
}

void GameRule::Join(Desk& desk,Player& user){
    ++desk.ready;
}

void GameRule::ChangeState(Desk& desk,Desk::State state){
    if(desk.state!=state){
        KEYE_LOG("----desk state: %d=>%d\n",desk.state,state);
        desk.state=state;
    }
}
