﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongRule: GameRule {

	public override int MaxCards{get{return 108;}}
	public override uint MaxPlayer{get{return 4;}}

	protected override void deal(MsgNCStart msg){
		uint id=0;
		for(uint k=1;k<=3;++k){
			for(uint i=1;i<=9;++i){
				for(uint j=0;j<4;++j){
					id=k*1000+j*100+i;
					Pile.Add(id);
				}
			}
		}
		//shuffle
		Pile=shuffle(Pile);
		/*
		//deal
		for(uint i=0;i<14;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<uint>[2]{new List<uint>(),new List<uint>()};
		for(uint i=14;i<14+13;++i)
			Hands[0].Add(Pile[i]);
		for(uint i=14+13;i<14+13*2;++i)
			Hands[1].Add(Pile[i]);
		*/
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		var bt=pb_enum.BunchA;
		return bt;
	}
	
	protected override bool compareBunch(bunch_t bunch,bunch_t hist){
		//rule win
		var win=true;
		return win;
	}
	
	public override int comparision(uint x,uint y){
		var cx=(int)x/1000;
		var cy=(int)y/1000;
		if(cx<cy)return 1;
		else if(cx==cy)return (int)y%100-(int)x%100;
		else return -1;
	}

	public override uint transformValue(uint val){
		return val;
	}
	
	public override uint inverseTransformValue(uint val){
		return val;
	}
}