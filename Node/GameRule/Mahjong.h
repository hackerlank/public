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
    virtual void        PostTick(Game&);
    virtual int         Type();
    virtual int         MaxPlayer();

    virtual bool        Ready(Game&);
    virtual void        Deal(Game&);
    virtual void        OnReady(Player&);
    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&);
    virtual void        OnMeld(Game&){};
    virtual bool        Settle(Game&);
    virtual bool        IsGameOver(Game&);
    
    virtual bool        Hint(Game&,pos_t,proto3::bunch_t&);
    
    static void         test();
private:
    proto3::pb_enum     verifyBunch(Game&,proto3::bunch_t&);
    bool                compareBunch(Game&,proto3::bunch_t&,proto3::bunch_t&);
    bool                comparision(Game&,uint x,uint y);
    void                log(Game&){}
    void                cards2str(Game&,std::string&,const google::protobuf::RepeatedField<uint32>&);
    void                logHands(Game&,uint32,std::string="");
    void                make_bunch(Game&,proto3::bunch_t&,const std::vector<uint>&);
};

#endif /* Mahjong_h */
