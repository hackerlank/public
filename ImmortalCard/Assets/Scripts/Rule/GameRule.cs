using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public abstract class GameRule {
	public List<uint> Pile;
	public List<uint>[] Hands;
	public List<bunch_t> Historical=new List<bunch_t>(); //historical game data

	public virtual int MaxCards{get{return 54;}}
	public virtual uint MaxPlayer{get{return 3;}}

	public List<bunch_t> Hint(uint[] hands,bunch_t src_bunch){
		var hints=new List<bunch_t>();
		if(hands!=null&&src_bunch!=null&&hands.Length>0){
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
					List<uint> cards=new List<uint>(hands);					//cards vector
					List<uint>[] sortByVal=new List<uint>[28];				//redundant vector
					for(int i=0;i<28;++i)sortByVal[i]=new List<uint>();
					foreach(var card in cards)sortByVal[card%100].Add(card);
					List<List<uint>>[] sortByWidth=new List<List<uint>>[5];	//null,A,AA,AAA,AAAA
					for(int i=0;i<5;++i)sortByWidth[i]=new List<List<uint>>();
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
							default:                    idx=1;break;
							}
							
							for(int j=idx;j<5;++j){
								var vv=sortByWidth[j];
								foreach(var v in vv){
									var card=v[0];
									if(card%100>histCard%100){
										var b=new bunch_t();
										b.Type=hist.Type;
										b.Pos=src_bunch.Pos+1;
										foreach(var c in v)b.Pawns.Add(c);
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
			
		}
		return hints;
	}

	public bool Verify(bunch_t curr,bunch_t last){
		return pb_enum.BunchInvalid!=verifyBunch(curr)&&compareBunch(curr,last);
	}

	public MsgNCStart Deal(){
		Pile=new List<uint>();

		var msg=new MsgNCStart();
		msg.Mid=pb_msg.MsgNcStart;
		msg.Banker=0;
		msg.Ante=10;
		msg.Multiple=1;

		deal(msg);
		return msg;
	}

	// pseudo-random number generator, using a time-dependent default seed value. 
	protected List<uint> shuffle(List<uint> list){
		System.Random random = new System.Random();
		for (int i = 0; i < list.Count; ++i){
			int var = random.Next(0, list.Count);
			uint temp = list[i];
			list[i] = list[var];
			list[var] = temp;
		}
		
		return list;
	}

	protected virtual void deal(MsgNCStart msg){}
	protected virtual pb_enum verifyBunch(bunch_t bunch){return pb_enum.BunchA;}
	protected virtual bool compareBunch(bunch_t bunch,bunch_t hist){return bunch.Type>hist.Type;}
	public abstract int comparision(uint x,uint y);
	public virtual uint transformValue(uint val){return val;}
	public virtual uint inverseTransformValue(uint val){return val;}
}
