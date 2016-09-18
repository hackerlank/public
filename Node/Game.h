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
    std::vector<Card>                   units;      //cards map
    std::vector<unit_id_t>              pile;       //cards library
    std::vector<Player*>                players;
    std::vector<proto3::game_data_t>    gameData;   //player game data
    std::vector<proto3::bunch_t>        historical; //historical game data
    
    int                                 ready;
    std::shared_ptr<GameRule>           rule;
    
    Game()
    :id(-1)
    ,ready(0)
    ,state(State::ST_WAIT)
    ,banker(0)
    ,token(0){}
};
#endif /* Game_hpp */
