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
    bool                Ready(Game&);
    void                ChangeState(Game&,Game::State);
    
    void                OnReady(Player&);

    virtual void        Tick(Game&)=0;
    virtual int         Type()=0;
    virtual int         MaxPlayer()=0;

    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&)=0;
    virtual void        OnMeld(Game&,Player&,const proto3::bunch_t&)=0;
protected:
    virtual void        initCard(Game&)=0;
    virtual int         maxCards()=0;
    virtual int         maxHands()=0;
    virtual int         bottom()=0;
    
    virtual bool        hint(google::protobuf::RepeatedField<proto3::bunch_t>&,Game&,pos_t,proto3::bunch_t&)=0;
    virtual bool        settle(Game&)=0;
    virtual bool        isGameOver(Game&)=0;
    
    virtual bool        comparision(Game&,uint x,uint y);
    void                deal(Game&);
    void                next(Game&);

    const char*         bunch2str(Game&,std::string&,const proto3::bunch_t&);
    const char*         cards2str(Game&,std::string&,const google::protobuf::RepeatedField<uint32>&);
    void                logHands(Game&,uint32,std::string="");
};

#endif /* GameRule_h */
