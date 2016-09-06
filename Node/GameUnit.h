//
//  GameUnit.hpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameUnit_hpp
#define GameUnit_hpp

struct GameUnit{
    unit_id_t   id;
    color_t     color;
    value_t     value;
    
    GameUnit()
    :id(0)
    ,color(0)
    ,value(0){}
};
typedef GameUnit Card;
typedef GameUnit Chess;

#endif /* GameUnit_hpp */
