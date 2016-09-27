using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongRule: GameRule {

	public override int MaxCards{get{return 108;}}
	public override uint MaxPlayer{get{return 4;}}

	protected override void deal(MsgNCStart msg){
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		var bt=pb_enum.BunchInvalid;
		return bt;
	}
	
	protected override bool compareBunch(bunch_t bunch,bunch_t hist){
		//rule win
		var win=false;
		return win;
	}
	
	public override int comparision(uint x,uint y){
		var cx=Configs.Cards[x];
		var cy=Configs.Cards[y];
		return ((int)cx.Value)-((int)cy.Value);
	}

	public override uint transformValue(uint val){
		if      (val==1) return 14;
		else if (val==2) return 16;
		else if (val==14)return 18;
		else if (val==15)return 19;
		else             return val;
	}
	
	public override uint inverseTransformValue(uint val){
		if      (val==14)return 1;
		else if (val==16)return 2;
		else if (val==18)return 14;
		else if (val==19)return 15;
		else             return val;
	}
}
