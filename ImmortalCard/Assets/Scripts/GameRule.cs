using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class GameRule {
	public int N=54;
	public uint[] Pile;
	public List<uint>[] Hands;

	public List<uint[]> Hint(uint[] hands,uint[] ids){
		var hints=new List<uint[]>();
		if(hands!=null&&ids!=null&&hands.Length>0&&ids.Length>0){

		}
		return hints;
	}

	public bool Verify(bunch_t curr,bunch_t last){
		return true;
	}

	public MsgNCStart Deal(){
		Pile=new uint[N];

		var msg=new MsgNCStart();
		msg.Mid=pb_msg.MsgNcStart;
		msg.Banker=0;
		msg.Ante=10;
		msg.Multiple=1;

		uint id=0;
		for(uint i=1;i<=13;++i){ //A-K => 1-13
			for(uint j=0;j<4;++j){
				Pile[id]=id;
				pawn_t card=new pawn_t();
				card.Color=j; //clubs,diamonds,hearts,spades => 0-3
				card.Value=i;
				card.Id=id++;
				msg.Cards.Add(card);
			}
		}
		for(uint j=0;j<=1;++j){  //Joker(color 0,1) => 14
			Pile[id]=id;
			pawn_t card=new pawn_t();
			card.Color=j;
			card.Value=14;
			card.Id=id++;
			msg.Cards.Add(card);
		}
		//shuffle
		Pile=Shuffle(Pile);
		//deal
		for(uint i=0;i<20;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<uint>[2]{new List<uint>(),new List<uint>()};
		for(uint i=20;i<20+17;++i)
			Hands[0].Add(Pile[i]);
		for(uint i=20+17;i<20+17*2;++i)
			Hands[1].Add(Pile[i]);

		return msg;
	}

	// pseudo-random number generator, using a time-dependent default seed value. 
	uint[] Shuffle(uint[] list){
		System.Random random = new System.Random();
		for (uint i = 0; i < list.Length; ++i){
			uint var = (uint)random.Next(0, list.Length);
			uint temp = list[i];
			list[i] = list[var];
			list[var] = temp;
		}
		
		return list;
	}
}
