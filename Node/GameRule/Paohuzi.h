//
//  Paohuzi.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Paohuzi_h
#define Paohuzi_h

class Paohuzi: public GameRule{
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
    bool                isGameOver(Game&,std::vector<int>&,std::vector<proto3::bunch_t>&);
    bool                hint3(Game&,pos_t,unit_id_t,proto3::bunch_t&);
    void                hint(Game&,unit_id_t,std::vector<int>&,std::vector<proto3::bunch_t>&);

    proto3::pb_enum     verifyBunch(Game&,proto3::bunch_t&);
    bool                comparePending(Game::pending_t& x,Game::pending_t& y);
    void                log(Game&){}
    void                make_bunch(proto3::bunch_t&,const std::vector<uint>&);
    bool				isNaturalWin(Game&,pos_t);
    void				draw(Game& game);
    
    int						winPoint(Game&,proto3::pb_enum);
    int						calcScore(Game&,proto3::pb_enum,int points);
    int						calcPoints(Game&,std::vector<proto3::bunch_t>&);
    int						calcPoints(Game&,pos_t);
    
    bool					chouWei(Game&,pos_t,proto3::bunch_t&);
    
};

#endif /* Paohuzi_h */
