//
//  Game.hpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Game_hpp
#define Game_hpp

class GameRule;
struct Game{
public:
    enum State{
        ST_WAIT,
        ST_START,
        ST_DISCARD,
        ST_MELD,
        ST_SETTLE,
        ST_END,
    };
    
    size_t      id;
    State       state;
    pos_t       banker;
    pos_t       token;
    size_t      currRound,maxRound;
    std::vector<Card>                   units;      //cards map
    std::vector<unit_id_t>              pile;       //cards library
    std::vector<std::shared_ptr<Player>> players;
    std::vector<proto3::game_data_t>    gameData;   //player game data
    std::vector<proto3::bunch_t>        historical; //historical game data
    
    int                                 ready;
    int                                 delay;
    std::shared_ptr<GameRule>           rule;
    
    Game()
    :id(-1)
    ,ready(0)
    ,delay(0)
    ,state(State::ST_WAIT)
    ,banker(0)
    ,token(0)
    ,currRound(0)
    ,maxRound(0){}
};
#endif /* Game_hpp */
