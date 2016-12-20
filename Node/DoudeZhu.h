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
    virtual int         MaxPlayer(Game&);

    static void         test();
protected:
    virtual void        initCard(Game&);
    virtual bool        validId(uint);
    virtual int         maxCards(Game& game);
    virtual int         maxHands(Game& game);
    virtual int         bottom(Game& game);
    
    virtual void        settle(Player&);
private:
    proto3::pb_enum     verifyBunch(proto3::bunch_t&)override;
    bool                compareBunch(proto3::bunch_t&,proto3::bunch_t&);
};

#endif /* DoudeZhu_h */
