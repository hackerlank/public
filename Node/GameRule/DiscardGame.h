//
//  DiscardGame.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DiscardGame_h
#define DiscardGame_h

class DiscardGame: public GameRule{
public:
    virtual void        Tick(Game&);

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Player&,const proto3::bunch_t&){};
protected:
    virtual bool        comparision(uint x,uint y);
    
    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&);
    virtual bool        settle(Game&);
private:
    virtual proto3::pb_enum verifyBunch(proto3::bunch_t&);
    virtual bool            compareBunch(proto3::bunch_t&,proto3::bunch_t&);
};

#endif /* DiscardGame_h */
