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
    for(auto i:_games){
        auto& game=*i.second;
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
                break;
            case Game::State::ST_MELD:
                break;
            case Game::State::ST_SETTLE:
                break;
            case Game::State::ST_END:
                break;
            default:
                break;
        }
    }
}

Game* GameRule::Create(Player& user,std::shared_ptr<Game> gameptr){
    if(gameptr){
        ++gameptr->ready;
        _games[gameptr->id]=gameptr;
    }
    return gameptr.get();
}

void GameRule::Join(Game& game,Player& user){
    ++game.ready;
}

void GameRule::ChangeState(Game& game,Game::State state){
    if(game.state!=state){
        KEYE_LOG("----game state: %d=>%d\n",game.state,state);
        game.state=state;
    }
}
