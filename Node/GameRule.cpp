//
//  GameRule.cpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#include "stdafx.h"
#include "NodeFwd.h"

void GameRule::Tick(Game& game){
    switch (game.state) {
        case Game::State::ST_WAIT:
            if(Ready(game))
                ChangeState(game,Game::State::ST_START);
            break;
        case Game::State::ST_START:
            Deal(game);
            ChangeState(game,Game::State::ST_DISCARD);
            break;
        case Game::State::ST_DISCARD:
            if(IsGameOver(game))
                ChangeState(game,Game::State::ST_SETTLE);
            break;
        case Game::State::ST_MELD:
            break;
        case Game::State::ST_SETTLE:
            Settle(game);
            ChangeState(game,Game::State::ST_END);
            break;
        case Game::State::ST_END:
            break;
        default:
            break;
    }
    PostTick(game);
}

void GameRule::PostTick(Game& game){
}

void GameRule::Next(Game& game){
    auto old=game.token;
    if(++game.token>=game.rule->MaxPlayer())game.token=0;
    KEYE_LOG("game token: %d=>%d\n",old,game.token);
}

void GameRule::ChangeState(Game& game,Game::State state){
    if(game.state!=state){
        KEYE_LOG("game state: %d=>%d\n",game.state,state);
        game.state=state;
    }
}
