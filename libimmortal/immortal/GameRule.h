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
    //----------------------------------------------------------------
    // shuld override
    //----------------------------------------------------------------
    //engage after deal
    virtual int         Type()=0;
    virtual int         MaxPlayer(Game&)=0;

    virtual bool        PreEngage(Game&,proto3::MsgNCEngage&)   {return true;}
    
    virtual bool        PreMeld(Game&)                          {return true;}
    //token: token before meld; front,current:bunches in pending meld
    virtual bool        PostMeld(Game&,proto3::pb_enum verifyBunchResult,pos_t token,proto3::bunch_t& front,
                                 proto3::bunch_t& current)      {return true;}
    
    virtual bool        PreDiscard(Game&,proto3::bunch_t&)      {return true;}
    virtual void        PostDiscard(Game&,proto3::MsgNCDiscard&){}
    
    virtual bool        PreSettle()     {return true;}
    virtual void        PostSettle()    {}
    
    //handle messages customized, no post message 'cause async handle
    virtual bool        PreMessage(Player&,PBHelper&)     {return true;}
protected:
    virtual void        initCard(Game&)=0;
    virtual proto3::pb_enum verifyBunch(proto3::bunch_t&)=0;
    virtual bool        validId(uint)=0;
    virtual bool        comparision(uint x,uint y)=0;
    virtual int         maxCards(Game& game)=0;
    virtual int         maxHands(Game& game)=0;
    virtual int         bottom(Game& game)=0;
    
    //----------------------------------------------------------------
    // override in Meld/DiscardGame
    //----------------------------------------------------------------
public:
    virtual             ~GameRule(){};
    virtual void        Tick(Game&)=0;
    virtual void        OnDiscard(Player&,proto3::MsgCNDiscard&)=0;
    virtual void        OnMeld(Player&,const proto3::bunch_t&)=0;
    
protected:
    virtual void        engage(Game&,proto3::MsgNCEngage&)=0;
    
    //----------------------------------------------------------------
    // functional
    //----------------------------------------------------------------
public:
    bool                IsReady(Game&);
    void                Release(Game&);
    void                OnReady(Player&);
    void                OnEngage(Player&,uint);

protected:
    void                settle(Game&);
    
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
