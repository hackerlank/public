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
		return pb_enum.BunchInvalid!=verifyBunch(curr)&&compareBunch(curr,last);
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
				card.Value=transformValue(i);
				card.Id=id++;
				msg.Cards.Add(card);
			}
		}
		for(uint j=0;j<=1;++j){  //Joker(color 0,1) => 14
			Pile[id]=id;
			pawn_t card=new pawn_t();
			card.Color=j;
			card.Value=transformValue(14);
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
	
	public pb_enum verifyBunch(bunch_t bunch){
		//sort cards
		List<uint> ids=new List<uint>(bunch.Pawns);
		ids.Sort(comparision);

		var len=ids.Count;
		var bt=pb_enum.BunchInvalid;
		List<pawn_t> cards=new List<pawn_t>();
		foreach(var c in ids)cards.Add(Configs.Cards[c]);
		//verify by length
		switch (len) {
		case 1:
		bt=pb_enum.BunchA;
			break;
		case 2:
			if(cards[0].Value==cards[1].Value){
				if(cards[0].Value==transformValue(14))
					// 2 Jokers
					bt=pb_enum.BunchAaaa;
				else
					bt=pb_enum.BunchAa;
			}
			break;
		case 3:
			if(cards[0].Value==cards[1].Value&&cards[0].Value==cards[2].Value)
				bt=pb_enum.BunchAaa;
			break;
		case 4:
			if(cards[0].Value==cards[1].Value&&cards[0].Value==cards[2].Value
			   &&cards[0].Value==cards[3].Value)
				bt=pb_enum.BunchAaaa;
			/*
            else if(cards[0].Value==cards[1].Value&&cards[2].Value==cards[3].Value
               &&cards[0].Value+1==cards[2].Value)
                bt=pb_enum.BunchAbc;  //AABB
            */
			else{
				//collect all counts
				Dictionary<uint,int> valCount=new Dictionary<uint, int>();  //[value,count]
				int maxSame=0;
				foreach(var card in cards){
					var val=card.Value;
					if(valCount.ContainsKey(val))
						valCount[val]++;
					else
						valCount[val]=1;
					if(valCount[val]>maxSame)
						maxSame=valCount[val];
				}
				if(maxSame==3&&valCount.Count==2)
					bt=pb_enum.BunchAaab;
			}
			break;
		default:{
			//more than 5: BunchAaab,BunchAbc,BunchAaaab
			Dictionary<uint,int> valCount=new Dictionary<uint, int>();  //[value,count]
			//collect all counts
			int maxSame=0;
			foreach(var card in cards){
				var val=card.Value;
				if(valCount.ContainsKey(val))
					valCount[val]++;
				else
					valCount[val]=1;
				if(valCount[val]>maxSame)
					maxSame=valCount[val];
			}
			switch (maxSame) {
			case 4:{
				List<int> counts=new List<int>();
				foreach(var imap in valCount){
					if(imap.Value!=4)
						counts.Add(imap.Value);
				};
				var lcounts=counts.Count;
				if(lcounts==1&&counts[0]==2){                  //AA
					bt=pb_enum.BunchAaaab;
				}else if(lcounts==2&&counts[0]==counts[1]){//AB,AABB,AAABBB
					bt=pb_enum.BunchAaaab;
				}
				break;
			}
			case 3:
				if(valCount.Count==2&&len<6)
					//only 1 AAA
					bt=pb_enum.BunchAaab;
				else{
					//more than 1: AAABBBCD,AAABBBCCCDEF
					List<int> B=new List<int>();
					List<uint> vAAA=new List<uint>();
					foreach(var imap in valCount){
						if(imap.Value!=3)
							B.Add(imap.Value);
						else
							vAAA.Add(imap.Key);
					};
					//adjacent check
					var AAA=vAAA.Count;
					var adjacent=true;
					for(int i=0;i<AAA-1;++i){
						if(vAAA[i]+1!=vAAA[i+1]){
							adjacent=false;
							break;
						}
					}
					if(adjacent){
						if(len-AAA==AAA*3||len==AAA*3)
							bt=pb_enum.BunchAaab;
						else if(AAA==B.Count){
							bt=pb_enum.BunchAaab;
							foreach(var m in B){
								foreach(var n in B){
									if(m!=n){
										//count A,B in AB not match
										bt=pb_enum.BunchInvalid;
										break;
									}
								}
								if(bt==pb_enum.BunchInvalid)
									break;
							}
						}
					}
				}
				break;
			case 2:
				bt=pb_enum.BunchAbc;
				if(len%2!=0){
					bt=pb_enum.BunchInvalid;
				}else for(int i=0;i<len-2;){
					if(cards[i].Value!=cards[i+1].Value)
						bt=pb_enum.BunchInvalid;
					else if(i+2<len&&cards[i].Value+1!=cards[i+2].Value)
						bt=pb_enum.BunchInvalid;
					i+=2;
				}
				break;
			case 1:
				bt=pb_enum.BunchAbc;
				for(int i=0;i<len-1;++i){
					if(cards[i].Value+1!=cards[i+1].Value){
						bt=pb_enum.BunchInvalid;
						break;
					}
				}
				break;
			default:
				break;
			}//switch
			break;
		}//default
		}//switch
		bunch.Type=bt;
		
		return bt;
	}
	
	public bool compareBunch(bunch_t bunch,bunch_t hist){
		//rule win
		var win=false;
		var bt=bunch.Type;
		if(bt==pb_enum.BunchInvalid){
			win=false;
		}else if(bt==pb_enum.BunchAaaa){
			//bomb
			var histCard=Configs.Cards[hist.Pawns[0]];
			var bunchCard=Configs.Cards[bunch.Pawns[0]];
			if(hist.Type!=pb_enum.BunchAaaa||histCard.Value<bunchCard.Value)
				win=true;
		}else if(bt==hist.Type&&bunch.Pawns.Count==hist.Pawns.Count){
			//same type and length
			switch (bt) {
			case pb_enum.BunchAaab:
			case pb_enum.BunchAaaab:{
				List<pawn_t> bunchCards=new List<pawn_t>();
				List<pawn_t> histCards=new List<pawn_t>();
				foreach(var c in bunch.Pawns)bunchCards.Add(Configs.Cards[c]);
				foreach(var c in hist.Pawns)histCards.Add(Configs.Cards[c]);
				//find value of the same cards
				uint bunchVal=0,histVal=0;
				if(pb_enum.BunchAaab==bt){
					for(int i=0,ii=bunchCards.Count-2;i<ii;++i){
						if(bunchCards[i].Value==bunchCards[i+1].Value&&bunchCards[i].Value==bunchCards[i+2].Value)
							bunchVal=bunchCards[i].Value;
					}
					for(int i=0,ii=histCards.Count-2;i<ii;++i){
						if(histCards[i].Value==histCards[i+1].Value&&histCards[i].Value==histCards[i+2].Value)
							histVal=histCards[i].Value;
					}
				}else{
					for(int i=0,ii=bunchCards.Count-3;i<ii;++i){
						if(bunchCards[i].Value==bunchCards[i+1].Value&&bunchCards[i].Value==bunchCards[i+2].Value
						   &&bunchCards[i].Value==bunchCards[i+3].Value)
							bunchVal=bunchCards[i].Value;
					}
					for(int i=0,ii=histCards.Count-3;i<ii;++i){
						if(histCards[i].Value==histCards[i+1].Value&&histCards[i].Value==histCards[i+2].Value
						   &&histCards[i].Value==histCards[i+3].Value)
							histVal=histCards[i].Value;
					}
				}
				win=bunchVal>histVal;
				break;
			}
			case pb_enum.BunchA:
			case pb_enum.BunchAa:
			case pb_enum.BunchAaa:
			case pb_enum.BunchAbc:
			default:{
				var histCard=Configs.Cards[hist.Pawns[0]];
				var bunchCard=Configs.Cards[bunch.Pawns[0]];
				if(histCard.Value<bunchCard.Value)
					win=true;
				break;
			}
			}
		}
		return win;
	}
	
	public int comparision(uint x,uint y){
		var cx=Configs.Cards[x];
		var cy=Configs.Cards[y];
		return ((int)cx.Value)-((int)cy.Value);
	}

	public uint transformValue(uint val){
		if      (val==1) return 14;
		else if (val==2) return 16;
		else if (val==14)return 18;
		else             return val;
	}
	
	public uint inverseTransformValue(uint val){
		if      (val==14)return 1;
		else if (val==16)return 2;
		else if (val==18)return 14;
		else             return val;
	}
}
