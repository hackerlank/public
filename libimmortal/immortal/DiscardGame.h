//
//  DiscardGame.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DiscardGame_h
#define DiscardGame_h

class KEYE_API DiscardGame: public GameRule{
protected:
protected:
    virtual bool            isGameOver(Game&);
    virtual bool            comparision(uint x,uint y);
    virtual void            settle(Player&)=0;
private:
    void                    Tick(Game&)override final;
    void                    OnDiscard(Player&,proto3::MsgCNDiscard&)override final;
    void                    OnMeld(Player&,const proto3::bunch_t&)override final{};
    
    void                    engage(Game&,proto3::MsgNCEngage&)override final;

    virtual proto3::pb_enum verifyBunch(proto3::bunch_t&)=0;
    virtual bool            compareBunch(proto3::bunch_t&,proto3::bunch_t&)=0;
};

#endif /* DiscardGame_h */
