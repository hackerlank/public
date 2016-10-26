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
        ST_ENGAGE,  //1
        ST_DISCARD,
        ST_MELD,    //3
        ST_SETTLE,
        ST_END,     //5
    };
    struct pending_t{
        //operation in oper queue
        proto3::bunch_t bunch;
        bool            arrived;
    };
    
    size_t      id;
    State       state;
    pos_t       banker;
    pos_t       token;
    size_t      round,Round;
    proto3::pb_enum                     category;
    std::string                         definedCards;
    std::vector<unit_id_t>              pile;       //cards library
    std::map<unit_id_t,int>             pileMap;    //cards map
    std::vector<std::shared_ptr<Player>> players;
    std::vector<proto3::bunch_t>        historical; //historical game data
    std::vector<pending_t>              pendingMeld;    //pending meld
    std::shared_ptr<pending_t>          pendingDiscard; //pending discard
    int                                 delay;
    std::shared_ptr<GameRule>           rule;
    
    std::shared_ptr<proto3::MsgNCSettle>    spSettle;
    std::shared_ptr<proto3::MsgNCFinish>    spFinish;
    
    //special for phz
    int     noWinner;
    bool    bankerChanged;

    Game()
    :id(-1)
    ,delay(0)
    ,state(State::ST_WAIT)
    ,banker(0)
    ,token(0)
    ,round(0)
    ,Round(1)
    ,noWinner(0)
    ,bankerChanged(false)
    {}
};
#endif /* Game_hpp */
