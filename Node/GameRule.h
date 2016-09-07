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
    Game*               Create(Player&,std::shared_ptr<Game>);
    void                Join(Game&,Player&);
    void                ChangeState(Game&,Game::State);
    
    virtual int         Type()=0;
    virtual int         MaxPlayer()=0;

    virtual bool        Ready(Game&)=0;
    virtual void        Deal(Game&)=0;
    virtual void        Settle(Game&)=0;
    virtual bool        IsGameOver(Game&)=0;
protected:
    std::map<game_id_t,std::shared_ptr<Game>> _games;
};

#endif /* GameRule_h */
