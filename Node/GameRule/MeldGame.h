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
    virtual bool            comparePending(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y);
    
    virtual void            settle(Player&,std::vector<proto3::bunch_t>&,unit_id_t)=0;

    virtual void            engage(Game&,proto3::MsgNCEngage&);
    virtual bool            meld(Game& game,Player&,unit_id_t,proto3::bunch_t&)=0;
    virtual void            draw(Game& game);
    virtual bool            checkDiscard(Player&,unit_id_t);  //check AAAA and AAA to decide discardable

    virtual void            onMeld(Game& game,Player&,unit_id_t,proto3::bunch_t&){};
private:
    //game over here
    virtual bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&)=0;
};

#endif /* MeldGame_h */
