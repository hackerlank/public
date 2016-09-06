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
    void                Tick();
    virtual void        Settle()=0;
    virtual bool        isGameOver()=0;
protected:
    std::map<int,std::shared_ptr<Desk>> _desks;
};

#endif /* GameRule_h */
