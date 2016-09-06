//
//  GameDesk.hpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameDesk_hpp
#define GameDesk_hpp

class Desk{
public:
    enum State{
        ST_WAIT,
        ST_START,
        ST_DISCARD,
        ST_MELD,
        ST_SETTLE,
        ST_END,
    };
    
    State   state;
    int     banker;
    int     token;
    std::vector<GameUnit>       units;
    std::vector<int>            pile;
    std::vector<user_time_t>    users;
    std::vector<GameData>       gameData;
    
};
#endif /* GameDesk_hpp */
