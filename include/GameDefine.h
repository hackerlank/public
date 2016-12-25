//
//  GameDefine.h
//  GameDefine
//
//  Created by Vic Liu on 8/26/16.
//  Copyright © 2016 Vic Liu. All rights reserved.
//

#ifndef GameDefine_h
#define GameDefine_h

enum TIMER:size_t{
    TIMER_SEC=100,
    TIMER_MIN,
    TIMER_HOUR,
    TIMER_DAY,
};

struct PaohuziData{
    //special for phz
    int     noWinner;
    bool    bankerChanged;
    int     m_winPeo;
    int		_multiScore;	//单局番数或者分数阈值
    int		_limitType;		//郴州，起手双提，控制能否继续吃碰，0：能，1：不能
    int		_fireDouble;	//广西跑胡子一炮双向。0未选择，1选择了
    
    PaohuziData(){
        memset(this,0,sizeof(*this));
    }
};

struct DoudeZhuData{
    //special for ddz
    int     anti;
    int     multiple;
    
    DoudeZhuData(){
        memset(this,0,sizeof(*this));
    }
};

#endif /* GameDefine_h */
