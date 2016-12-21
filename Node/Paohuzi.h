//
//  Paohuzi.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Paohuzi_h
#define Paohuzi_h

class Paohuzi: public MeldGame{
    //----------------------------------------------------------------
    // overrode
    //----------------------------------------------------------------
public:
    bool            PreEngage(Game&,proto3::MsgNCEngage&)override;
    bool            PostMeld(Game& game,proto3::pb_enum,pos_t token,proto3::bunch_t&,proto3::bunch_t&)override;
    bool            PreSettle(Player&,std::vector<proto3::bunch_t>*,unit_id_t)override;
    void            PostSettle(Game&)override;

    int             Type()override;
    int             MaxPlayer(Game& game)override;
    
    bool            comparePendingMeld(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y)override;
    static void     test();
protected:
    void            initCard(Game&)override;
    bool            validId(uint)override;
    int             maxCards(Game& game)override;
    int             maxHands(Game& game)override;
    int             bottom(Game& game)override;

private:
    void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>&)override;
    //is game over with melt card
    bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&)override;
    //check AAAA and AAA to decide discardable
    bool            canDiscard(Player&,unit_id_t)override;
    proto3::pb_enum verifyBunch(proto3::bunch_t&)override;
    void            PostDraw(Game& game,unit_id_t)override;
    
    //----------------------------------------------------------------
    // functional
    //----------------------------------------------------------------
    bool            meld(Game& game,pos_t token,proto3::bunch_t& front,proto3::bunch_t&);
    void            calcAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&);
    void            calcPengAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&,int);
    int				winPoint(Game&,proto3::pb_enum);
    int				calcScore(Game&,proto3::pb_enum,int points);
    int				calcPoints(Game&,std::vector<proto3::bunch_t>&);
    int             calcMultiOrScore(Game&,int);
    
    bool			chouWei(Game&,Player&,proto3::bunch_t&);

    int             findSuiteKT(Game&,std::vector<unit_id_t> hands,int type,int pos);

};

#endif /* Paohuzi_h */
