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
        ST_DRAW,
        ST_DISCARD,
        ST_MELD,
        ST_SETTLE,
        ST_END,
    };
    struct pending_t{
        //operation in oper queue
        pos_t           pos;
        proto3::pb_enum ops;
        unit_id_t       card;
        bool            arrived;
                        pending_t():pos(i_invalid),ops(proto3::pb_enum::BUNCH_INVALID),card(i_invalid),arrived(false){}
    };
    
    size_t      id;
    State       state;
    pos_t       banker;
    pos_t       token;
    size_t      round,Round;
    std::vector<Card>                   units;      //cards map
    std::vector<unit_id_t>              pile;       //cards library
    std::vector<std::shared_ptr<Player>> players;
    std::vector<proto3::bunch_t>        historical; //historical game data
    std::vector<pending_t>              pendingMeld;    //pending meld
    std::shared_ptr<pending_t>          pendingDiscard; //pending discard
    int                                 delay;
    std::shared_ptr<GameRule>           rule;
    
    Game()
    :id(-1)
    ,delay(0)
    ,state(State::ST_WAIT)
    ,banker(0)
    ,token(0)
    ,round(0)
    ,Round(1){}
};
#endif /* Game_hpp */
