using UnityEngine;
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

	public override List<bunch_t> Hint(Player player,uint[] hands,bunch_t src_bunch){
		//for meld: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
		var hints=new List<bunch_t>();
		if(player==null||hands==null||src_bunch==null||hands.Length<=0)
			return hints;

		uint pos=player.pos;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return hints;
		}
		var id=src_bunch.Pawns[0];
		var A=id;

		//default color check
		if(A/1000==player.gameData.SelectedCard){
			Debug.Log("hint default color,pos="+pos);
			return hints;
		}

		//game over
		List<bunch_t> output=new List<bunch_t>();
		if(isGameOver(player,id,output)){
			var bunch=new bunch_t();
			bunch.Pos=pos;
			bunch.Type=pb_enum.BunchWin;
			bunch.Pawns.Add(id);
			hints.Add(bunch);
		}
		
		//select color
		List<uint> sel=new List<uint>();
		foreach(var hand in hands){
			var B=hand;
			if(B/1000==A/1000&&B%100==A%100)
				sel.Add(hand);
		}
		var len=sel.Count;
		if(len>=2){
			if(len>=3){
				//BUNCH_AAAA
				var bunch=new bunch_t();
				bunch.Pos=pos;
				bunch.Type=pb_enum.BunchAaaa;
				for(int i=0;i<3;++i)bunch.Pawns.Add(sel[i]);
				bunch.Pawns.Add(id);
				hints.Add(bunch);
			}
			if(src_bunch.Pos!=pos){
				//BUNCH_AAA, not for self
				var bunch=new bunch_t();
				bunch.Pos=pos;
				bunch.Type=pb_enum.BunchAaa;
				for(int i=0;i<2;++i)bunch.Pawns.Add(sel[i]);
				bunch.Pawns.Add(id);
				hints.Add(bunch);
			}
		}else if(src_bunch.Pos==pos){
			//BUNCH_AAAA, only for self
			foreach(var melt in player.gameData.Bunch){
				if(melt.Type==pb_enum.BunchAaa){
					var C=melt.Pawns[0];
					if(C/1000==A/1000&&C%100==A%100){
						//BUNCH_AAAA
						var bunch=new bunch_t();
						bunch.Pos=pos;
						bunch.Type=pb_enum.BunchAaaa;
						bunch.Pawns.AddRange(melt.Pawns);
						bunch.Pawns.Add(id);
						hints.Add(bunch);
						break;
					}
				}
			}
		}
		
		var count=hints.Count;
		if(count>0){
			string str="hint "+count+",pos="+pos+",";
			foreach(var bunch in hints)
				str+=Player.bunch2str(bunch);
			Debug.Log(str);
		}
		return hints;
	}

	bool isGameOver(Player player,uint id,List<bunch_t> output){
		var hands=player.gameData.Hands;
		if(hands.Count<2){
			Debug.Log("isGameOver failed: len="+hands.Count);
			return false;
		}
		List<uint> cards=new List<uint>();
		cards.AddRange(hands);
		cards.Add(id);
		cards.Sort(Main.Instance.gameController.Rule.comparision);

		var len=cards.Count-1;
		for(int i=0;i!=len;++i){
			var A=cards[i+0];
			var B=cards[i+1];
			if(A/1000==B/1000&&A%100==B%100){
				List<uint> tmp=new List<uint>();
				for(int j=0;j!=cards.Count;++j)if(j!=i&&j!=i+1)tmp.Add(cards[j]);
				if(isGameOverWithoutAA(tmp))
					return true;
			}
		}
		return false;
	}
	
	bool isGameOverWithoutAA(List<uint> cards){
		var len=cards.Count;
		if(len%3!=0){
			//Debug.Log("isGameOverWithoutAA failed: len=%lu\n",len);
			return false;
		}
		
		for(int i=0,ii=len/3;i!=ii;++i){
			var A=cards[i+0];
			var B=cards[i+1];
			var C=cards[i+2];
			if(A/1000!=B/1000||A/1000!=C/1000){
				//Debug.Log("isGameOverWithoutAA failed: color\n");
				return false;
			}
			if(!((A%100+1==B%100&&A%100+2==C%100)||(A%100==B%100&&A%100==C%100))){
				//Debug.Log("isGameOverWithoutAA failed: invalid pattern\n");
				return false;
			}
		}
		return true;
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

	public static uint FindDefaultColor(Player player){
		int key=20;
		int I=0;
		int[] count=new int[3];
		foreach(var card in player.gameData.Hands)count[card/1000-1]++;
		for(int i=0;i<3;++i){
			if(key>count[i]){
				key=count[i];
				I=i;
			}
		}
		key=1000*(I+1)+1;
		return (uint)key;
	}
}
