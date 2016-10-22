//
//  Mahjong.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Mahjong_h
#define Mahjong_h

class Mahjong: public MeldGame{
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
    
    virtual bool            hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&);

    virtual bool            verifyDiscard(Game&,proto3::bunch_t&);
    virtual proto3::pb_enum verifyBunch(Game&,proto3::bunch_t&);
    //is game over with melt card
    virtual bool            isGameOver(Game&,pos_t,unit_id_t,std::vector<proto3::bunch_t>&);
    virtual bool            isGameOver(Game&,std::vector<unit_id_t>&,std::vector<proto3::bunch_t>&);
    //is game over against cards without AA
    bool                    isGameOverWithoutAA(std::vector<unit_id_t>&);

    virtual proto3::pb_enum meld(Game& game,pos_t,unit_id_t,proto3::bunch_t&);
private:
    virtual void            deal(Game&);
};

#endif /* Mahjong_h */
