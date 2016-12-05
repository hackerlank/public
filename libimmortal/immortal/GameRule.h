//
//  GameRule.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameRule_h
#define GameRule_h

class KEYE_API GameRule{
public:
    virtual             ~GameRule(){};
    bool                Ready(Game&);

    virtual void        Tick(Game&)=0;
    virtual int         Type()=0;
    virtual int         MaxPlayer(Game&)=0;

    void                OnEngage(Player&,uint);
    void                OnReady(Player&);
    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&)=0;
    virtual void        OnMeld(Player&,const proto3::bunch_t&)=0;
protected:
    virtual void        initCard(Game&)=0;
    virtual bool        validId(uint)=0;
    virtual bool        comparision(uint x,uint y)=0;
    virtual int         maxCards(Game& game)=0;
    virtual int         maxHands(Game& game)=0;
    virtual int         bottom(Game& game)=0;
    
    virtual bool        settle(Game&);
    void                release(Game&);
    
    virtual void        engage(Game&,proto3::MsgNCEngage&){};
    void                deal(Game&);
    void                changeState(Game&,Game::State);
    void                changePos(Game&,pos_t);

    const char*         state2str(std::string&,Game::State);
    const char*         bunch2str(std::string&,const proto3::bunch_t&);
    const char*         cards2str(std::string&,const google::protobuf::RepeatedField<int>&);
    void                logHands(Game&,uint32,std::string="");
    
    void                persistReplay(Game&);
    void                make_bunch(proto3::bunch_t&,const std::vector<uint>&);
};

#endif /* GameRule_h */
