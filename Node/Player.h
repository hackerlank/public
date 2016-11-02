//
//  Player.hpp
//  Node
//
//  Created by Vic Liu on 9/7/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Player_hpp
#define Player_hpp

struct Game;
class Player: public std::enable_shared_from_this<Player>{
public:
                            Player(keye::svc_handler& sh);
    int                     getKey();
    void                    reset();
    
    void                    send(google::protobuf::MessageLite& msg);
    void                    on_read(PBHelper&);
    
    pos_t                   pos;
    bool                    ready,engaged;
    std::shared_ptr<Game>   game;
    proto3::play_t                                  playData;   //gameplay data
    std::shared_ptr<google::protobuf::MessageLite>  lastMsg;
    
    //special for phz
    std::vector<proto3::bunch_t>    AAAAs,AAAs;
    std::vector<unit_id_t>          unpairedCards,discardedCards,dodgeCards;   //past and abandon cards
    bool    conflictMeld;       //冲招
    int     inputCount;         //draw count
    int     lastHand;           //last card in hand
    int     winCount;           //win by continuous
    int     m_winMark;          //peng huzi win bunch type
    
    bool			m_bczArr;		//郴州，标示起手提是否>1--true:>1 false:<=1
    bool			m_bdoubleTi;	//所有玩法，起手双提(重跑)标示，true:双提，false：0或1提
    
private:
    std::shared_ptr<keye::svc_handler>              spsh;
};
#endif /* Player_hpp */
