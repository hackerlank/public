//
//  GameRule.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameRule_h
#define GameRule_h

class GameRule{
public:
    virtual             ~GameRule(){};
    void                Deal(Game&);
    bool                Ready(Game&);
    void                Next(Game&);
    void                ChangeState(Game&,Game::State);
    
    void                OnReady(Player&);

    virtual void        Tick(Game&)=0;
    virtual int         Type()=0;
    virtual int         MaxPlayer()=0;
    virtual int         MaxCards()=0;
    virtual int         MaxHands()=0;
    virtual int         Bottom()=0;

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&)=0;
    virtual void        OnMeld(Game&,Player&,const proto3::bunch_t&)=0;
    virtual bool        Hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&)=0;
    virtual bool        Settle(Game&)=0;
    virtual bool        IsGameOver(Game&)=0;
protected:
    virtual void        initCard(Game&)=0;
    virtual bool        comparision(Game&,uint x,uint y)=0;
    
    const char*         bunch2str(Game&,std::string&,const proto3::bunch_t&);
    const char*         cards2str(Game&,std::string&,const google::protobuf::RepeatedField<uint32>&);
    void                logHands(Game&,uint32,std::string="");
};

#endif /* GameRule_h */
