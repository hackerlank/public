//
//  GameDesk.hpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameDesk_hpp
#define GameDesk_hpp

struct Desk{
public:
    enum State{
        ST_WAIT,
        ST_START,
        ST_DISCARD,
        ST_MELD,
        ST_SETTLE,
        ST_END,
    };
    
    State       state;
    pos_t       banker;
    pos_t       token;
    std::vector<GameUnit>       units;
    std::vector<unit_id_t>      pile;
    std::vector<proto3::user_t> users;
    std::vector<GameData>       gameData;
    
    int                         ready;
    
    Desk()
    :ready(0)
    ,state(State::ST_WAIT)
    ,banker(0)
    ,token(0){}
};
#endif /* GameDesk_hpp */
