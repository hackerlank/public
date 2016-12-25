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
    //----------------------------------------------------------------
    // overrode
    //----------------------------------------------------------------
public:
    bool            PreEngage(Game&,proto3::MsgNCEngage&)override;
    bool            PostMeld(Game& game,proto3::pb_enum,pos_t token,proto3::bunch_t&,proto3::bunch_t&)override;
    bool            PreDiscard(Game&,proto3::bunch_t&)override;
    bool            PreSettle(Player&,std::vector<proto3::bunch_t>*,unit_id_t)override;

    int             Type()override;
    int             MaxPlayer(Game& game)override;

    static void     test();
protected:
    void            initCard(Game&)override;
    bool            validId(unsigned)override;
    int             maxCards(Game& game)override;
    int             maxHands(Game& game)override;
    int             bottom(Game& game)override;
    //check hands to decide discardable
    bool            canDiscard(Player&,unit_id_t)override;
    proto3::pb_enum verifyBunch(proto3::bunch_t&)override;
private:
    void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>&)override;
    //is game over with melt card
    bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&)override;

    //----------------------------------------------------------------
    // functional
    //----------------------------------------------------------------
    bool            meld(Game& game,pos_t token,proto3::bunch_t& front,proto3::bunch_t&);
    void            calcAchievement(Player&,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&);
};

#endif /* Mahjong_h */
