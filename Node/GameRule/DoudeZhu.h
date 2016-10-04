//
//  DoudeZhu.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DoudeZhu_h
#define DoudeZhu_h

class DoudeZhu: public GameRule{
public:
    virtual void        Tick(Game&);
    virtual int         Type();
    virtual int         MaxPlayer();

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Game&,Player&,const proto3::bunch_t&){};
    
    static void         test();
protected:
    virtual void        initCard(Game&);
    virtual int         maxCards();
    virtual int         maxHands();
    virtual int         bottom();
    
    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&);
    virtual bool        settle(Game&);
    virtual bool        isGameOver(Game&);
private:
    void                tickRobot(Game&);
    proto3::pb_enum     verifyBunch(Game&,proto3::bunch_t&);
    bool                compareBunch(Game&,proto3::bunch_t&,proto3::bunch_t&);
    bool                comparision(Game&,uint x,uint y);
    void                log(Game&){}
    void                make_bunch(Game&,proto3::bunch_t&,const std::vector<uint>&);
};

#endif /* DoudeZhu_h */
