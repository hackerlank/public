using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuRule: GameRule {

	public override int MaxCards{get{return 54;}}
	public override uint MaxPlayer{get{return 3;}}

	protected override void deal(MsgNCStart msg){
		uint id=0;
		for(uint i=1;i<=13;++i){ //A-K => 1-13
			for(uint j=1;j<=4;++j){
				id=j*1000+transformValue(i);
				Pile.Add(id);
			}
		}
		for(uint j=1;j<=2;++j){  //Joker(color 0,1) => 14,15
			id=j*1000+transformValue(14+j);
			Pile.Add(id);
		}
		//shuffle
		Pile=shuffle(Pile);
		//deal
		for(int i=0;i<20;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<uint>[2]{new List<uint>(),new List<uint>()};
		for(int i=20;i<20+17;++i)
			Hands[0].Add(Pile[i]);
		for(int i=20+17;i<20+17*2;++i)
			Hands[1].Add(Pile[i]);
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		//sort cards
		List<uint> ids=new List<uint>(bunch.Pawns);
		ids.Sort(comparision);

		var len=ids.Count;
		var bt=pb_enum.BunchInvalid;
		List<uint> cards=new List<uint>();
		foreach(var c in ids)cards.Add(c);
		//verify by length
		switch (len) {
		case 1:
		bt=pb_enum.BunchA;
			break;
		case 2:
			if(cards[0]%100==cards[1]%100)
				bt=pb_enum.BunchAa;
			else if(cards[0]%100>=transformValue(14)&&cards[1]%100>=transformValue(14))
				// 2 Jokers
				bt=pb_enum.BunchAaaa;
			break;
		case 3:
			if(cards[0]%100==cards[1]%100&&cards[0]%100==cards[2]%100)
				bt=pb_enum.BunchAaa;
			break;
		case 4:
			if(cards[0]%100==cards[1]%100&&cards[0]%100==cards[2]%100
			   &&cards[0]%100==cards[3]%100)
				bt=pb_enum.BunchAaaa;
			/*
            else if(cards[0]%100==cards[1]%100&&cards[2]%100==cards[3]%100
               &&cards[0]%100+1==cards[2]%100)
                bt=pb_enum.BunchAbc;  //AABB
            */
			else{
				//collect all counts
				Dictionary<uint,int> valCount=new Dictionary<uint, int>();  //[value,count]
				int maxSame=0;
				foreach(var card in cards){
					var val=card%100;
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
				var val=card%100;
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
					if(cards[i]%100!=cards[i+1]%100)
						bt=pb_enum.BunchInvalid;
					else if(i+2<len&&cards[i]%100+1!=cards[i+2]%100)
						bt=pb_enum.BunchInvalid;
					i+=2;
				}
				break;
			case 1:
				bt=pb_enum.BunchAbc;
				for(int i=0;i<len-1;++i){
					if(cards[i]%100+1!=cards[i+1]%100){
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
	
	protected override bool compareBunch(bunch_t bunch,bunch_t hist){
		//rule win
		var win=false;
		var bt=bunch.Type;
		if(bt==pb_enum.BunchInvalid){
			win=false;
		}else if(bt==pb_enum.BunchAaaa){
			//bomb
			var histCard=hist.Pawns[0];
			var bunchCard=bunch.Pawns[0];
			if(hist.Type!=pb_enum.BunchAaaa||histCard%100<bunchCard%100)
				win=true;
		}else if(bt==hist.Type&&bunch.Pawns.Count==hist.Pawns.Count){
			//same type and length
			switch (bt) {
			case pb_enum.BunchAaab:
			case pb_enum.BunchAaaab:{
				List<uint> bunchCards=new List<uint>();
				List<uint> histCards=new List<uint>();
				foreach(var c in bunch.Pawns)bunchCards.Add(c);
				foreach(var c in hist.Pawns)histCards.Add(c);
				//find value of the same cards
				uint bunchVal=0,histVal=0;
				if(pb_enum.BunchAaab==bt){
					for(int i=0,ii=bunchCards.Count-2;i<ii;++i){
						if(bunchCards[i]%100==bunchCards[i+1]%100&&bunchCards[i]%100==bunchCards[i+2]%100)
							bunchVal=bunchCards[i]%100;
					}
					for(int i=0,ii=histCards.Count-2;i<ii;++i){
						if(histCards[i]%100==histCards[i+1]%100&&histCards[i]%100==histCards[i+2]%100)
							histVal=histCards[i]%100;
					}
				}else{
					for(int i=0,ii=bunchCards.Count-3;i<ii;++i){
						if(bunchCards[i]%100==bunchCards[i+1]%100&&bunchCards[i]%100==bunchCards[i+2]%100
						   &&bunchCards[i]%100==bunchCards[i+3]%100)
							bunchVal=bunchCards[i]%100;
					}
					for(int i=0,ii=histCards.Count-3;i<ii;++i){
						if(histCards[i]%100==histCards[i+1]%100&&histCards[i]%100==histCards[i+2]%100
						   &&histCards[i]%100==histCards[i+3]%100)
							histVal=histCards[i]%100;
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
				var histCard=hist.Pawns[0];
				var bunchCard=bunch.Pawns[0];
				if(histCard%100<bunchCard%100)
					win=true;
				break;
			}
			}
		}
		return win;
	}
	
	public override int comparision(uint x,uint y){
		return (int)y%100-(int)x%100;
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
