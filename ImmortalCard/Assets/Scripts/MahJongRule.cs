using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongRule: GameRule {

	public override int MaxCards{get{return 108;}}
	public override uint MaxPlayer{get{return 4;}}

	protected override void deal(MsgNCStart msg){
		uint id=0;
		for(uint i=1;i<=13;++i){ //A-K => 1-13
			for(uint j=0;j<4;++j){
				Pile[id]=id;
				pawn_t card=new pawn_t();
				card.Color=j; //clubs,diamonds,hearts,spades => 0-3
				card.Value=transformValue(i);
				card.Id=id++;
				msg.Cards.Add(card);
			}
		}
		for(uint j=0;j<=1;++j){  //Joker(color 0,1) => 14,15
			Pile[id]=id;
			pawn_t card=new pawn_t();
			card.Color=j;
			card.Value=transformValue(14+j);
			card.Id=id++;
			msg.Cards.Add(card);
		}
		//shuffle
		Pile=shuffle(Pile);
		//deal
		for(uint i=0;i<20;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<uint>[2]{new List<uint>(),new List<uint>()};
		for(uint i=20;i<20+17;++i)
			Hands[0].Add(Pile[i]);
		for(uint i=20+17;i<20+17*2;++i)
			Hands[1].Add(Pile[i]);
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
