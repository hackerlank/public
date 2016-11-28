//
//  Mahjong.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Mahjong_h
#define Mahjong_h

class Mahjong: public MeldGame{
public:
    virtual int             Type();
    virtual int             MaxPlayer(Game& game);

    static void             test();
protected:
    virtual void            initCard(Game&);
    virtual bool            validId(uint);
    virtual int             maxCards(Game& game);
    virtual int             maxHands(Game& game);
    virtual int             bottom(Game& game);
    virtual void            settle(Player&,std::vector<proto3::bunch_t>&,unit_id_t);
    virtual void            engage(Game&,proto3::MsgNCEngage&);
    
    virtual void            onMeld(Game& game,Player&,unit_id_t,proto3::bunch_t&);

    virtual bool            checkDiscard(Player&,unit_id_t);  //check AAAA and AAA to decide discardable
    virtual bool            verifyDiscard(Game&,proto3::bunch_t&);
    virtual proto3::pb_enum verifyBunch(Game&,proto3::bunch_t&);
private:
    virtual void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>&);
    //is game over with melt card
    virtual bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&);

    virtual bool            meld(Game& game,Player&,unit_id_t,proto3::bunch_t&);
    
    void                    calcAchievement(Player&,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&);
};

#endif /* Mahjong_h */