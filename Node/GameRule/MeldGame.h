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
    virtual void            Tick(Game&);

    virtual void            OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void            OnMeld(Player&,const proto3::bunch_t&);
protected:
    virtual bool            verifyDiscard(Game&,proto3::bunch_t&){return true;}
    virtual proto3::pb_enum verifyBunch(Game&,proto3::bunch_t&)=0;

    virtual bool            comparision(uint x,uint y);
    virtual bool            comparePending(Game::pending_t& x,Game::pending_t& y);
    
    //is game over with melt card
    virtual bool            isGameOver(Game&,Player&,unit_id_t,std::vector<proto3::bunch_t>&)=0;
    virtual bool            isGameOver(Game&,std::vector<unit_id_t>&,std::vector<proto3::bunch_t>&)=0;

    virtual bool            hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,Player&,proto3::bunch_t&)=0;

    virtual void            deal(Game&);
    virtual void            meld(Game& game,Player&,unit_id_t,proto3::bunch_t&)=0;
    virtual void            draw(Game& game);
    virtual bool            isNaturalWin(Game&,Player&);
};

#endif /* MeldGame_h */
