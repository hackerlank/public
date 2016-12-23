//
//  Immortal.h
//  Immortal
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Immortal_h
#define Immortal_h

class KEYE_API Immortal :public Server {
public:
                    Immortal();
    virtual void	on_open(keye::svc_handler&);
    virtual void	on_close(keye::svc_handler&);
    virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
    virtual void	on_write(keye::svc_handler&, void*, size_t sz);
    virtual bool	on_timer(keye::svc_handler&, size_t id, size_t milliseconds);
    
    void            addPlayer(size_t,std::shared_ptr<Player>);

    void            registerRule(std::shared_ptr<GameRule>);
    
    std::shared_ptr<Game>   createGame(int,proto3::MsgCNCreate&);
    std::shared_ptr<Game>   findGame(game_id_t);
    void                    removeGame(game_id_t);
    
    static Immortal*        sImmortal;
private:
    std::map<size_t,std::shared_ptr<Player>>    players;
    std::map<int,std::shared_ptr<GameRule>>     gameRules;
    std::map<size_t,std::shared_ptr<Game>>      games;

    int                 _game_index;
};

#endif /* Immortal_h */
