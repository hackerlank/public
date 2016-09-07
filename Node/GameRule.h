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
    void                Tick();
    Desk*               Create(Player&,std::shared_ptr<Desk>);
    void                Join(Desk&,Player&);
    void                ChangeState(Desk&,Desk::State);
    
    virtual int         Type()=0;
    virtual int         MaxPlayer()=0;

    virtual bool        Ready(Desk&)=0;
    virtual void        Deal(Desk&)=0;
    virtual void        Settle(Desk&)=0;
    virtual bool        IsGameOver(Desk&)=0;
protected:
    std::map<desk_id_t,std::shared_ptr<Desk>> _desks;
};

#endif /* GameRule_h */
