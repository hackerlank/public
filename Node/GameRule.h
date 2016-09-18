//
//  GameRule.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameRule_h
#define GameRule_h

class GameRule{
public:
    virtual             ~GameRule(){};
    void                Tick(Game&);
    void                ChangeState(Game&,Game::State);
    
    virtual void        PostTick(Game&);
    virtual int         Type()=0;
    virtual int         MaxPlayer()=0;

    virtual bool        Ready(Game&)=0;
    virtual void        Deal(Game&)=0;
    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&)=0;
    virtual void        OnMeld(Game&)=0;
    virtual void        Settle(Game&)=0;
    virtual bool        IsGameOver(Game&)=0;
};

#endif /* GameRule_h */
