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
    virtual int         Type();
    virtual int         MaxPlayer();

    virtual bool        Ready(Desk&);
    virtual void        Deal(Desk&);
    virtual void        Settle(Desk&);
    virtual bool        IsGameOver(Desk&);
};

#endif /* DoudeZhu_h */
