//
//  MeldGame.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef MeldGame_h
#define MeldGame_h

class MeldGame: public GameRule{
public:
    virtual void        Tick(Game&);

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Player&,const proto3::bunch_t&);
protected:
    virtual bool        settle(Game&);
    
    virtual bool        verifyDiscard(Game&,proto3::bunch_t&);
    virtual proto3::pb_enum verifyBunch(Game&,proto3::bunch_t&);

    virtual bool        comparision(uint x,uint y);
    virtual bool        comparePending(Game::pending_t& x,Game::pending_t& y);
    
    //is game over with melt card
    virtual bool        isGameOver(Game&,pos_t,unit_id_t,std::vector<proto3::bunch_t>&);
    //is game over against cards without AA
    bool                isGameOverWithoutAA(std::vector<unit_id_t>&);

    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&)=0;
private:
    void				draw(Game& game);
    bool				isNaturalWin(Game&,pos_t);
};

#endif /* MeldGame_h */
