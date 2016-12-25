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
    bool            PreSettle(Player&,std::vector<proto3::bunch_t>*,unit_id_t)override;

    int             Type()override;
    int             MaxPlayer(Game&)override;

    static void     test();
protected:
    void            init(Game&)override;
    void            initCard(Game&)override;
    bool            validId(unsigned)override;
    int             maxCards(Game& game)override;
    int             maxHands(Game& game)override;
    int             bottom(Game& game)override;
private:
    proto3::pb_enum verifyBunch(proto3::bunch_t&)override;
    bool            compareBunch(proto3::bunch_t&,proto3::bunch_t&)override;
};

#endif /* DoudeZhu_h */
