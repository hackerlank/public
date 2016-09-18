//
//  Player.hpp
//  Node
//
//  Created by Vic Liu on 9/7/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef Player_hpp
#define Player_hpp

struct Game;
class Player: public std::enable_shared_from_this<Player>{
public:
    Player(keye::svc_handler& sh);
    int     getKey();
    
    void    send(google::protobuf::MessageLite& msg);
    void    on_read(PBHelper&);
    
    pos_t                   pos;
    std::shared_ptr<Game>   game;
private:
    std::shared_ptr<keye::svc_handler>  spsh;
};
#endif /* Player_hpp */
