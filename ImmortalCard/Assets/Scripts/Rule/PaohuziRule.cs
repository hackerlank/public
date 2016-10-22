using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziRule: GameRule {

	public override int MaxCards{get{return 80;}}
	public override int MaxPlayer{get{return 3;}}

	protected override void deal(MsgNCStart msg){
		int id=0;
		for(int k=1;k<=3;++k){
			for(int i=1;i<=9;++i){
				for(int j=0;j<4;++j){
					id=k*1000+j*100+i;
					Pile.Add(id);
				}
			}
		}
		//shuffle
		Pile=shuffle(Pile);
		/*
		//deal
		for(int i=0;i<14;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<int>[2]{new List<int>(),new List<int>()};
		for(int i=14;i<14+13;++i)
			Hands[0].Add(Pile[i]);
		for(int i=14+13;i<14+13*2;++i)
			Hands[1].Add(Pile[i]);
		*/
	}

	public override List<bunch_t> Hint(Player player,bunch_t src_bunch){
		//for meld: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
		var hints=new List<bunch_t>();
		var hands=player.playData.Hands;
		if(player==null||src_bunch==null||hands.Count<=0)
			return hints;

		int pos=player.pos;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return hints;
		}
		var id=src_bunch.Pawns[0];
		var A=id;

		//default color check
		if(A/1000==player.playData.SelectedCard){
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
		List<int> sel=new List<int>();
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
			foreach(var melt in player.playData.Bunch){
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

	bool isGameOver(Player player,int id,List<bunch_t> output){
		var hands=player.playData.Hands;
		if(hands.Count<2){
			Debug.Log("isGameOver failed: len="+hands.Count);
			return false;
		}
		List<int> cards=new List<int>();
		cards.AddRange(hands);
		cards.Add(id);
		cards.Sort(Main.Instance.gameController.Rule.comparision);

		var len=cards.Count-1;
		for(int i=0;i!=len;++i){
			var A=cards[i+0];
			var B=cards[i+1];
			if(A/1000==B/1000&&A%100==B%100){
				List<int> tmp=new List<int>();
				for(int j=0;j!=cards.Count;++j)if(j!=i&&j!=i+1)tmp.Add(cards[j]);
				if(isGameOverWithoutAA(tmp))
					return true;
			}
		}
		return false;
	}
	
	bool isGameOverWithoutAA(List<int> cards){
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
		return pb_enum.BunchA;
	}
	
	public override int comparision(int x,int y){
		var cx=(int)x/1000;
		var cy=(int)y/1000;
		if(cx<cy)return 1;
		else if(cx==cy)return (int)y%100-(int)x%100;
		else return -1;
	}

	public override int transformValue(int val){
		return val;
	}
	
	public override int inverseTransformValue(int val){
		return val;
	}

	public static int FindDefaultColor(Player player){
		int key=20;
		int I=0;
		int[] count=new int[3];
		foreach(var card in player.playData.Hands)count[card/1000-1]++;
		for(int i=0;i<3;++i){
			if(key>count[i]){
				key=count[i];
				I=i;
			}
		}
		key=1000*(I+1)+1;
		return key;
	}
	
	public override PlayerController AIController{
		get{
			if(aiController==null)aiController=new PaohuziAIController();
			return aiController;
		}
	}
}
