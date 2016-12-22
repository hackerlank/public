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
struct KEYE_API Game{
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
    
    int         id;
    State       state;
    long        start_timestamp;
    long        dismiss_timestamp;
    pos_t       banker;
    pos_t       token;
    int         round,Round;
    unit_id_t   firstCard,lastCard;
    int         dismissRequest;
    
    proto3::pb_enum                     category;
    std::string                         definedCards;
    std::vector<unit_id_t>              pile;       //cards library
    std::vector<unit_id_t>              bottom;     //bottom cards
    std::map<unit_id_t,int>             pileMap;    //cards map
    std::vector<proto3::bunch_t>        historical; //historical game data
    std::vector<pending_t>              pendingMeld;    //pending meld
    std::shared_ptr<pending_t>          pendingDiscard; //pending discard
    std::shared_ptr<proto3::replay_item>    spReplayItem;
    std::shared_ptr<proto3::replay_data>    spReplay;   //record for replaying
    std::shared_ptr<proto3::MsgNCSettle>    spSettle;
    std::shared_ptr<proto3::MsgNCFinish>    spFinish;
    
    std::shared_ptr<GameRule>               rule;
    std::vector<std::shared_ptr<Player>>    players;
    std::shared_ptr<void*>                  spExtra;    //custom data

    //special for phz
    int     noWinner;
    bool    bankerChanged;
    int     m_winPeo;
    int		_multiScore;	//单局番数或者分数阈值
    int		_limitType;		//郴州，起手双提，控制能否继续吃碰，0：能，1：不能
    int		_fireDouble;	//广西跑胡子一炮双向。0未选择，1选择了
    
    //special for ddz
    int     anti;
    int     multiple;
    
    Game()
    :id(-1)
    ,dismissRequest(0)
    ,dismiss_timestamp(0)
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
