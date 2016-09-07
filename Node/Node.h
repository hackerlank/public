//
//  Node.h
//  Node
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Node_h
#define Node_h

class Node :public keye::ws_service {
public:
                    Node(size_t ios = 1, size_t works = 1, size_t rb_size = 510);
    virtual void	on_open(keye::svc_handler&);
    virtual void	on_close(keye::svc_handler&);
    virtual void	on_read(keye::svc_handler& sh, void* buf, size_t sz);
    virtual void	on_write(keye::svc_handler&, void*, size_t sz);
    virtual bool	on_timer(keye::svc_handler&, size_t id, size_t milliseconds);
    
    void            registerGame(std::shared_ptr<GameRule>);
    GameRule*       findGame(int);
    std::shared_ptr<Game>   createGame(proto3::MsgCNCreate&);
    void            removeGame(game_id_t);
    
    static Node*            sNode;
private:
    std::map<size_t,std::shared_ptr<Player>>    players;
    std::map<int,std::shared_ptr<GameRule>>     gameRules;
    
    size_t                  _game_index;
};

#endif /* Node_h */
