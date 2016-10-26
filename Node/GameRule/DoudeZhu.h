//
//  DoudeZhu.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DoudeZhu_h
#define DoudeZhu_h

class DoudeZhu: public DiscardGame{
public:
    virtual int         Type();
    virtual int         MaxPlayer();

    static void         test();
protected:
    virtual void        initCard(Game&);
    virtual bool        validId(uint);
    virtual int         maxCards();
    virtual int         maxHands();
    virtual int         bottom();
    
    virtual void        settle(Player&);
    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,Player&,proto3::bunch_t&);
private:
    proto3::pb_enum     verifyBunch(proto3::bunch_t&);
    bool                compareBunch(proto3::bunch_t&,proto3::bunch_t&);
};

#endif /* DoudeZhu_h */
