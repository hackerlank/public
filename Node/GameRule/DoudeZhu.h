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
    virtual void        OnMeld(Player&,const proto3::bunch_t&){};
    
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
    //void                tickRobot(Game&);
    proto3::pb_enum     verifyBunch(proto3::bunch_t&);
    bool                compareBunch(proto3::bunch_t&,proto3::bunch_t&);
    void                log(Game&){}
    void                make_bunch(proto3::bunch_t&,const std::vector<uint>&);
};

#endif /* DoudeZhu_h */
