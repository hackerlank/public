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
    //----------------------------------------------------------------
    // shuld override
    //----------------------------------------------------------------
protected:
    bool                    comparision(unsigned x,unsigned y)override;
    //sort pending meld
    virtual void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>& output);
private:
    //precheck while draw
    virtual bool            PreDraw(Game&)  {return true;}
    //post draw process
    virtual void            PostDraw(Game&,unit_id_t card) {}

    //game over here
    virtual bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&)=0;
    //sort function for pending meld
    virtual bool            comparePendingMeld(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y);
    //check discardable after draw or meld, will pass to the next or draw one more if failed
    virtual bool            canDiscard(Player&,unit_id_t)=0;
    //----------------------------------------------------------------
    // overrode
    //----------------------------------------------------------------
private:
    void                    Tick(Game&)override final;
    void                    OnDiscard(Player&,proto3::MsgCNDiscard&)override final;
    void                    OnMeld(Player&,const proto3::bunch_t&)override final;
    
    void                    engage(Game&,proto3::MsgNCEngage&)override final;
    //----------------------------------------------------------------
    // functional
    //----------------------------------------------------------------
    //draw and send to client
    void                    draw(Game& game);
protected:
    //prepare server for accepting cient discard
    void                    discard(Player&,unit_id_t);
};

#endif /* MeldGame_h */
