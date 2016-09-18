//
//  GameData.hpp
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameData_hpp
#define GameData_hpp

typedef short unit_id_t;
typedef short pos_t;
typedef short color_t;
typedef short value_t;
typedef unsigned game_id_t;

struct GameData{
    std::vector<unit_id_t>    hands;
    std::vector<unit_id_t>    discards;
    
    void clear(){
        hands.clear();
        discards.clear();
    }
};
#endif /* GameData_hpp */
