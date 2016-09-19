//
//  DoudeZhu.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DoudeZhu_h
#define DoudeZhu_h

class DoudeZhu: public GameRule{
public:
    virtual void        PostTick(Game&);
    virtual int         Type();
    virtual int         MaxPlayer();

    virtual bool        Ready(Game&);
    virtual void        Deal(Game&);
    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Game&){};
    virtual void        Settle(Game&);
    virtual bool        IsGameOver(Game&);
    
    virtual bool        Hint(Game&,pos_t,proto3::bunch_t&);
private:
    bool                verifyDiscard(Game&,proto3::bunch_t&,proto3::bunch_t&);
    void                log(Game&){}
    void                logHands(Game&,uint32);
};

#endif /* DoudeZhu_h */
