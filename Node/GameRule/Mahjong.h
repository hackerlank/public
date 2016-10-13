//
//  Mahjong.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Mahjong_h
#define Mahjong_h

class Mahjong: public GameRule{
public:
    virtual void        Tick(Game&);
    virtual int         Type();
    virtual int         MaxPlayer();

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Player&,const proto3::bunch_t&);
    
    static void         test();
protected:
    virtual void        initCard(Game&);
    virtual bool        validId(uint);
    virtual bool        comparision(uint x,uint y);
    virtual int         maxCards();
    virtual int         maxHands();
    virtual int         bottom();
    
    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&);
    virtual bool        settle(Game&);
    virtual bool        isGameOver(Game&);
private:
    //is game over with melt card
    bool                isGameOver(Game&,pos_t,unit_id_t,std::vector<proto3::bunch_t>&);
    //is game over against cards without AA
    bool                isGameOverWithoutAA(std::vector<unit_id_t>&);
    virtual void        deal(Game&);

    proto3::pb_enum     verifyBunch(Game&,proto3::bunch_t&);
    bool                verifyDiscard(Game&,proto3::bunch_t&);
    bool                comparePending(Game::pending_t& x,Game::pending_t& y);
    void                log(Game&){}
    void                make_bunch(proto3::bunch_t&,const std::vector<uint>&);
    bool				isNaturalWin(Game&,pos_t);
    void				draw(Game& game);
};

#endif /* Mahjong_h */
