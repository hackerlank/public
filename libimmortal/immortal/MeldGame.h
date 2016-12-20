//
//  MeldGame.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef MeldGame_h
#define MeldGame_h

class KEYE_API MeldGame: public GameRule{
protected:
    virtual bool            comparePending(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y);

    virtual bool            comparision(uint x,uint y);
    
    virtual void            settle(Player&,std::vector<proto3::bunch_t>&,unit_id_t)=0;

    virtual void            draw(Game& game);
    virtual bool            checkDiscard(Player&,unit_id_t);  //check AAAA and AAA to decide discardable

    virtual void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>&);
private:
    void                    Tick(Game&)override final;
    void                    OnDiscard(Player&,proto3::MsgCNDiscard&)override final;
    void                    OnMeld(Player&,const proto3::bunch_t&)override final;
    
    void                    engage(Game&,proto3::MsgNCEngage&)override final;
    //game over here
    virtual bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&)=0;
};

#endif /* MeldGame_h */
