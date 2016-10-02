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
    virtual int         MaxCards();
    virtual int         MaxHands();
    virtual int         Bottom();

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Game&,Player&,const proto3::bunch_t&);
    virtual bool        Settle(Game&);
    virtual bool        IsGameOver(Game&);
    
    virtual bool        Hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&);
    
    static void         test();
protected:
    virtual void        initCard(Game&);
private:
    void                tickRobot(Game&);
    proto3::pb_enum     verifyBunch(Game&,proto3::bunch_t&);
    bool                compareBunch(Game&,proto3::bunch_t&,proto3::bunch_t&);
    bool                comparision(Game&,uint x,uint y);
    void                log(Game&){}
    void                make_bunch(Game&,proto3::bunch_t&,const std::vector<uint>&);
    bool				isNaturalWin(Game&,pos_t);
    void				draw(Game& game);
    void				meld(Game&,unit_id_t,pos_t);
};

#endif /* Mahjong_h */