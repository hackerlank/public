using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuRule: GameRule {

	public override int MaxCards{get{return 54;}}
	public override int MaxPlayer{get{return 3;}}

	protected override void deal(MsgNCStart msg){
		int id=0;
		for(int i=1;i<=13;++i){ //A-K => 1-13
			for(int j=1;j<=4;++j){
				id=j*1000+transformValue(i);
				Pile.Add(id);
			}
		}
		for(int j=1;j<=2;++j){  //Joker(color 0,1) => 14,15
			id=j*1000+transformValue(14+j);
			Pile.Add(id);
		}
		//shuffle
		Pile=shuffle(Pile);
		//deal
		for(int i=0;i<20;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<int>[2]{new List<int>(),new List<int>()};
		for(int i=20;i<20+17;++i)
			Hands[0].Add(Pile[i]);
		for(int i=20+17;i<20+17*2;++i)
			Hands[1].Add(Pile[i]);
	}

	public override List<bunch_t> Hint(Player player,bunch_t src_bunch){
		var hints=new List<bunch_t>();
		var hands=player.playData.Hands;
		if(src_bunch!=null&&hands.Count>0){
			var H=Historical.Count;
			if(H<=0){
				var b=new bunch_t();
				b.Type=pb_enum.BunchA;
				b.Pos=src_bunch.Pos+1;
				b.Pawns.Add(hands[0]);
				hints.Add(b);
			}else{
				bunch_t hist=Historical[H-1];
				if(hist.Type==pb_enum.OpPass&&H>1)
					hist=Historical[H-2];
				var type=(pb_enum)hist.Type;
				Debug.Log("Hint by "+Player.bunch2str(hist));
				if(type==pb_enum.OpPass){
					var b=new bunch_t();
					b.Type=pb_enum.BunchA;
					b.Pos=src_bunch.Pos+1;
					b.Pawns.Add(hands[0]);
					hints.Add(b);
				}else if(hist.Pawns.Count>0){
					List<int> cards=new List<int>(hands);					//cards vector
					List<int>[] sortByVal=new List<int>[28];				//redundant vector
					for(int i=0;i<28;++i)sortByVal[i]=new List<int>();
					foreach(var card in cards)sortByVal[card%100].Add(card);
					List<List<int>>[] sortByWidth=new List<List<int>>[5];	//null,A,AA,AAA,AAAA
					for(int i=0;i<5;++i)sortByWidth[i]=new List<List<int>>();
					foreach(var sorted in sortByVal)sortByWidth[sorted.Count].Add(sorted);
					
					var histCard=hist.Pawns[0];
					if(type==pb_enum.BunchAbc){
						//make a queue without duplicated
						cards.Clear();
						foreach(var v in sortByVal)if(v.Count>0&&v[0]%100>histCard%100)cards.Add(v[0]);
						if(cards.Count>0){
							int len=(int)hist.Pawns.Count;
							int y=(int)cards.Count-len;
							for(int i=0;i<y;++i){
								bunch_t bunch_=new bunch_t();
								for(int j=i,jj=i+len;j!=jj;++j)bunch_.Pawns.Add(cards[j]);
								var bt=verifyBunch(bunch_);
								if(bt==type&&compareBunch(bunch_,hist)){
									bunch_.Pos=src_bunch.Pos+1;
									hints.Add(bunch_);
								}
							}
						}
					}else{
						switch(hist.Type){
						case pb_enum.BunchA:
						case pb_enum.BunchAa:
						case pb_enum.BunchAaa:
						case pb_enum.BunchAaaa:{
							int idx=1;
							switch(hist.Type){
							case pb_enum.BunchAaaa:   idx=4;break;
							case pb_enum.BunchAaa:    idx=3;break;
							case pb_enum.BunchAa:     idx=2;break;
							case pb_enum.BunchA:
							default:                  idx=1;break;
							}
							
							for(int j=idx;j<5;++j){
								var vv=sortByWidth[j];
								foreach(var v in vv){
									var card=v[0];
									if(card%100>histCard%100){
										var b=new bunch_t();
										b.Type=hist.Type;
										b.Pos=src_bunch.Pos+1;
										for(int i=0;i<idx;++i)b.Pawns.Add(v[i]);	//overflow!
										hints.Add(b);
									}
								}
							}
							break;
						}
						case pb_enum.BunchAaaab:
							if(sortByWidth[4].Count>0&&sortByWidth[1].Count>=2){
								var id0=sortByWidth[1][0][0];
								var id1=sortByWidth[1][1][0];
								foreach(var sorted in sortByWidth[4]){
									bunch_t bunch_=new bunch_t();
									bunch_.Pawns.Add(id0);
									bunch_.Pawns.Add(id1);
									foreach(var card in sorted)bunch_.Pawns.Add(card);
									var bt=verifyBunch(bunch_);
									if(bt==type&&compareBunch(bunch_,hist)){
										bunch_.Pos=src_bunch.Pos+1;
										bunch_.Type=hist.Type;
										hints.Add(bunch_);
									}
								}
							}
							break;
						case pb_enum.BunchAaab:
							if(sortByWidth[3].Count>0&&sortByWidth[1].Count>0){
								var id=sortByWidth[1][0][0];
								foreach(var sorted in sortByWidth[3]){
									bunch_t bunch_=new bunch_t();
									bunch_.Pawns.Add(id);
									foreach(var card in sorted)bunch_.Pawns.Add(card);
									var bt=verifyBunch(bunch_);
									if(bt==type&&compareBunch(bunch_,hist)){
										bunch_.Pos=src_bunch.Pos+1;
										bunch_.Type=hist.Type;
										hints.Add(bunch_);
									}
								}
							}
							break;
						default:
							break;
						}//switch
					}//else if(type==pb_enum.BUNCH_ABC)
					if(hist.Type!=pb_enum.BunchAaaa&&sortByWidth[4].Count>0){
						//boom!
						foreach(var sorted in sortByWidth[4]){
							bunch_t bunch_=new bunch_t();
							foreach(var card in sorted)bunch_.Pawns.Add(card);
							bunch_.Pos=src_bunch.Pos+1;
							bunch_.Type=pb_enum.BunchAaaa;
							hints.Add(bunch_);
						}
					}
				}//else if(type==pb_enum.OpPass)
			}//else if(H<=0)
		}//if(hands!=null&&src_bunch!=null&&hands.Length>0)
		return hints;
	}

	public override bool verifyDiscard(Player player,bunch_t bunch){
		//discard my card
		if(player.pos!=Token%MaxPlayer){
			Debug.Log("Discard invalid turn");
			return false;
		}
		
		if(Historical.Count>0){
			var hist=Historical[Historical.Count-1];
			if(hist.Type==pb_enum.OpPass&&Historical.Count>=2)
				hist=Historical[Historical.Count-2];
			var check=(hist.Type==pb_enum.OpPass||
			       pb_enum.BunchInvalid!=verifyBunch(bunch)
			       &&compareBunch(bunch,hist));
			if(!check){
				Debug.Log("Discard invalid bunch");
				return false;
			}
		}
		return true;
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		//sort cards
		List<int> ids=new List<int>(bunch.Pawns);
		ids.Sort(comparision);

		var len=ids.Count;
		var bt=pb_enum.BunchInvalid;
		List<int> cards=new List<int>();
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
				Dictionary<int,int> valCount=new Dictionary<int, int>();  //[value,count]
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
			Dictionary<int,int> valCount=new Dictionary<int, int>();  //[value,count]
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
					List<int> vAAA=new List<int>();
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
				List<int> bunchCards=new List<int>();
				List<int> histCards=new List<int>();
				foreach(var c in bunch.Pawns)bunchCards.Add(c);
				foreach(var c in hist.Pawns)histCards.Add(c);
				//find value of the same cards
				int bunchVal=0,histVal=0;
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
	
	public override int comparision(int x,int y){
		return (int)y%100-(int)x%100;
	}

	public override int transformValue(int val){
		if      (val==1) return 14;
		else if (val==2) return 16;
		else if (val==14)return 18;
		else if (val==15)return 19;
		else             return val;
	}
	
	public override int inverseTransformValue(int val){
		if      (val==14)return 1;
		else if (val==16)return 2;
		else if (val==18)return 14;
		else if (val==19)return 15;
		else             return val;
	}

	public override PlayerController AIController{
		get{
			if(aiController==null)aiController=new DoudeZhuAIController();
			return aiController;
		}
	}
}
