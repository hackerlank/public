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
    virtual int             Type();
    virtual int             MaxPlayer();
    
    static void             test();
protected:
    virtual void            initCard(Game&);
    virtual bool            validId(uint);
    virtual int             maxCards();
    virtual int             maxHands();
    virtual int             bottom();
    virtual void            settle(Player&,std::vector<proto3::bunch_t>&,unit_id_t);
    virtual void            engage(Game&,proto3::MsgNCEngage&);
private:
    //is game over with melt card
    virtual bool            isWin(Game&,Player&,unit_id_t,std::vector<proto3::bunch_t>&);
    bool                    isWin(Game&,std::vector<unit_id_t>&,std::vector<proto3::bunch_t>&);
    bool                    hint3(Game&,Player&,unit_id_t,proto3::bunch_t&);
    void                    hint(Game&,unit_id_t,std::vector<unit_id_t>&,std::vector<proto3::bunch_t>&);

    virtual bool            comparePending(std::shared_ptr<Game>,Game::pending_t& x,Game::pending_t& y);
    virtual bool            checkDiscard(Player&,unit_id_t);  //check AAAA and AAA to decide discardable
    proto3::pb_enum         verifyBunch(Game&,proto3::bunch_t&);
    virtual bool            meld(Game& game,Player&,unit_id_t,proto3::bunch_t&);
    
    void                    calcAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&);
    void                    calcPengAchievement(Game&,proto3::pb_enum,const std::vector<proto3::bunch_t>&,std::vector<proto3::achv_t>&,int);
    int						winPoint(Game&,proto3::pb_enum);
    int						calcScore(Game&,proto3::pb_enum,int points);
    int						calcPoints(Game&,std::vector<proto3::bunch_t>&);
    int						calcPoints(Game&,Player&);
    
    bool					chouWei(Game&,Player&,proto3::bunch_t&);

    int                     findSuiteKT(Game&,std::vector<unit_id_t> hands,int type,int pos);

};

#endif /* Paohuzi_h */
