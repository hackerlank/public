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
public:
    bool            PreEngage(Game&,proto3::MsgNCEngage&)override;
    bool            PostMeld(Game& game,proto3::pb_enum,pos_t token,proto3::bunch_t&,proto3::bunch_t&)override;

    virtual int             Type();
    virtual int             MaxPlayer(Game& game);
    
    bool                    comparePending(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y)override;
    static void             test();
protected:
    virtual void            initCard(Game&);
    virtual bool            validId(uint);
    virtual int             maxCards(Game& game);
    virtual int             maxHands(Game& game);
    virtual int             bottom(Game& game);
    virtual void            settle(Player&,std::vector<proto3::bunch_t>&,unit_id_t);

private:
    virtual void            sortPendingMeld(std::shared_ptr<Game>,std::vector<proto3::bunch_t>&);
    //is game over with melt card
    virtual bool            isWin(Game&,proto3::bunch_t&,std::vector<proto3::bunch_t>&);

    virtual bool            checkDiscard(Player&,unit_id_t);  //check AAAA and AAA to decide discardable
    proto3::pb_enum         verifyBunch(proto3::bunch_t&)override;
    virtual bool            meld(Game& game,pos_t token,proto3::bunch_t& front,proto3::bunch_t&);
    virtual void            draw(Game& game);
    
    void                    calcAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&);
    void                    calcPengAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&,int);
    int						winPoint(Game&,proto3::pb_enum);
    int						calcScore(Game&,proto3::pb_enum,int points);
    int						calcPoints(Game&,std::vector<proto3::bunch_t>&);
    int                     calcMultiOrScore(Game&,int);
    
    bool					chouWei(Game&,Player&,proto3::bunch_t&);

    int                     findSuiteKT(Game&,std::vector<unit_id_t> hands,int type,int pos);

};

#endif /* Paohuzi_h */
