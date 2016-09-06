//
//  DoudeZhu.h
//  Node
//
//  Created by Vic Liu on 9/6/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef DoudeZhu_h
#define DoudeZhu_h

class DoudeZhu: public GameRule{
public:
    virtual int         type();
    virtual int         maxPlayer();
    virtual void        settle();
    virtual bool        isGameOver();
protected:
};

#endif /* DoudeZhu_h */
