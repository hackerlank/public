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
class Player{
public:
    Player(keye::svc_handler& sh);
    
    void    on_read(PBHelper&);
private:
    std::shared_ptr<keye::svc_handler>  spsh;
    std::shared_ptr<Game>               game;
};
#endif /* Player_hpp */
