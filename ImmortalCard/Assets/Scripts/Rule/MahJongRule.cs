using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongRule: GameRule {

	public override int MaxCards{get{return 108;}}
	public override int MaxPlayer{get{return 4;}}

	protected override void deal(MsgNCDeal msg){
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

		int pos=player.playData.Seat;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return hints;
		}
		var A=src_bunch.Pawns[0];

		//draw with AAAA,merge them
		var bunch=new bunch_t();
		if(src_bunch.Type==pb_enum.BunchA){
			if(player.AAAAs.Count>0){
				bunch.Pos=pos;
				bunch.Type=pb_enum.BunchA;
				bunch.Pawns.Add(A);
				bunch.Child.Add(player.AAAAs);
				hints.Add(bunch);
			}
		}

		//default color check
		if(A/1000==player.playData.SelectedCard){
			Debug.Log("hint default color,pos="+pos);
			return hints;
		}

		//game over
		bunch=isWin(player,A);
		if(bunch!=null)
			hints.Add(bunch);
		
		//normal check
		if(A>1000){
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
					bunch=new bunch_t();
					bunch.Pawns.Add(A);
					bunch.Type=pb_enum.BunchAaaa;
					for(int i=0;i<3;++i)bunch.Pawns.Add(sel[i]);
					hints.Add(bunch);
				}
				if(src_bunch.Pos!=pos){
					//BUNCH_AAA, not for self
					bunch=new bunch_t();
					bunch.Pawns.Add(A);
					bunch.Type=pb_enum.BunchAaa;
					for(int i=0;i<2;++i)bunch.Pawns.Add(sel[i]);
					hints.Add(bunch);
				}
			}else if(src_bunch.Pos==pos){
				//BUNCH_AAAA, only for self
				foreach(var melt in player.playData.Bunch){
					if(melt.Type==pb_enum.BunchAaa){
						var C=melt.Pawns[0];
						if(C/1000==A/1000&&C%100==A%100){
							//BUNCH_AAAA
							bunch=new bunch_t();
							bunch.Pawns.Add(A);
							bunch.Type=pb_enum.BunchAaaa;
							bunch.Pawns.AddRange(melt.Pawns);
							hints.Add(bunch);
							break;
						}
					}
				}
			}
		}
		foreach(var hint in hints)hint.Pos=player.playData.Seat;

		var count=hints.Count;
		if(count>0){
			string str=count+" hints for "+pos+": ";
			foreach(var bun in hints)
				str+=Player.bunch2str(bun);
			Debug.Log(str);
		}
		return hints;
	}

	public override void Meld(Player player,bunch_t bunch){
		List<bunch_t> melds=new List<bunch_t>();
		switch(bunch.Type){
		case pb_enum.BunchA:
			player.playData.Hands.Add(bunch.Pawns[0]);
			//draw with AAAA
			melds.AddRange(bunch.Child);
			break;
		case pb_enum.BunchAaa:
		case pb_enum.BunchAaaa:
			melds.Add(bunch);
			break;
		}
		foreach(var meld in melds){
			player.playData.Bunch.Add(meld);
			foreach(var card in meld.Pawns){
				player.playData.Hands.Remove(card);
			}
			var A=meld.Pawns[0];
			foreach(var a in player.AAAAs){
				var B=a.Pawns[0];
				if(B/1000==A/1000 && B%100==A%100){
					player.AAAAs.Remove(a);
					break;
				}
			}
		}
	}
	
	public bunch_t isWin(Player player,int card){
		var hands=player.playData.Hands;
		if(hands.Count<2)
			return null;

		List<int> cards=new List<int>(hands);
		if(card!=Config.invalidCard){
			var inhand=false;
			foreach(var i in cards)if(i==card){inhand=true;break;}
			if(!inhand)cards.Add(card);
		}
		cards.Sort(comparision);

		var win=false;
		var bunches=new List<bunch_t>();
		while(cards.Count==14){
			//13 Orphans

			if(win)break;

			//7 pairs
			win=true;
			for(int i=0;i<7;++i){
				var A=cards[i*2];
				var B=cards[i*2+1];
				if(A/1000!=B/1000||A%100!=B%100){
					win=false;
					break;
				}
			}
			if(win){
				for(int i=0;i<7;++i){
					var A=cards[i*2];
					var B=cards[i*2+1];
					var bunch=new bunch_t();
					bunch.Pawns.Add(A);
					bunch.Pawns.Add(B);
					bunch.Type=pb_enum.BunchAa;
					
					if(i<7-1){
						var C=cards[i*2+2];
						if(C/1000==B/1000||C%100==B%100){
							bunch.Pawns.Add(cards[i*2+2]);
							bunch.Pawns.Add(cards[i*2+3]);
							bunch.Type=pb_enum.BunchAaaa;
							++i;
						}
					}
					
					bunches.Add(bunch);
				}
				cards.Clear();
			}
			break;
		}

		//generic
		var len=cards.Count-1;
		for(int i=0;i<len;++i){
			var A=cards[i+0];
			var B=cards[i+1];
			if(A/1000==B/1000&&A%100==B%100){
				List<int> tmp=new List<int>();
				//exclude AA
				for(int j=0;j!=cards.Count;++j)if(j!=i&&j!=i+1)tmp.Add(cards[j]);
				if(isWinWithoutAA(tmp,bunches)){
					win=true;

					var aa=new bunch_t();
					aa.Type=pb_enum.BunchAa;
					aa.Pawns.Add(A);
					aa.Pawns.Add(B);
					bunches.Add(aa);

					foreach(var bunch in player.playData.Bunch)bunches.Add(bunch);
					break;
				}
			}
		}
		if(win){
			var output=new bunch_t();
			output.Type=pb_enum.BunchWin;
			output.Pos=player.playData.Seat;
			output.Pawns.Add(card);
			foreach(var bunch in bunches)output.Child.Add(bunch);
			return output;
		}
		return null;
	}
	
	bool isWinWithoutAA(List<int> cards,List<bunch_t> output){
		var len=cards.Count;
		if(len%3!=0)
			return false;

		int i=0;
		while(i<len){
			//next 3 continuous cards
			var A=cards[i+0];
			var B=cards[i+1];
			var C=cards[i+2];
			
			if(A/1000 == B/1000 && A/1000 == C/1000){
				//same color
				A%=100;B%=100;C%=100;
				
				if((A+1==B && B+1==C) || (A==B && A==C)){
					//great values
					var bunch=new bunch_t();
					bunch.Type=(A==B && A==C?pb_enum.BunchAaa:pb_enum.BunchAbc);
					for(int j=0;j<3;++j)bunch.Pawns.Add(cards[i+j]);
					output.Add(bunch);

					i+=3;
					continue;
				}else if(i+6<=len){
					//next 6 continuous cards
					var D=cards[i+3];
					var E=cards[i+4];
					var F=cards[i+5];

					if(D/1000 == E/1000 && D/1000 == F/1000){
						//type AABBCC
						D%=100;E%=100;F%=100;
						if(A==B && C==D && E==F && B+1==C && D+1==E){
							//great values
							var bunch=new bunch_t();
							bunch.Type=pb_enum.BunchAbc;
							for(int j=0;j<3;++j)bunch.Pawns.Add(cards[i+j*2]);
							output.Add(bunch);

							bunch=new bunch_t();
							bunch.Type=pb_enum.BunchAbc;
							for(int j=0;j<3;++j)bunch.Pawns.Add(cards[i+j*2+1]);
							output.Add(bunch);

							i+=6;
							continue;
						}
					}else if(D/1000 == E/1000 && D/1000 == F/1000){
						//type ABBCCD
						D%=100;E%=100;F%=100;
						if(B==C && D==E && A+1==B && E+1==F){
							//great values
							var bunch=new bunch_t();
							bunch.Type=pb_enum.BunchAbc;
							bunch.Pawns.Add(cards[i+0]);
							bunch.Pawns.Add(cards[i+1]);
							bunch.Pawns.Add(cards[i+3]);
							output.Add(bunch);
							
							bunch=new bunch_t();
							bunch.Type=pb_enum.BunchAbc;
							bunch.Pawns.Add(cards[i+2]);
							bunch.Pawns.Add(cards[i+4]);
							bunch.Pawns.Add(cards[i+5]);
							output.Add(bunch);
							
							i+=6;
							continue;
						}
					}
				}
			}
			//other wise
			output.Clear();
			return false;
		}
		
		return true;
	}

	public override bool verifyDiscard(Player player,bunch_t bunch){
		if(!base.verifyDiscard(player,bunch))return false;

		//huazhu check
		if(bunch.Pawns.Count<=0)return false;

		var B=player.playData.SelectedCard;
		var A=bunch.Pawns[0];
		if(A/1000!=B/1000){
			foreach(var card in player.playData.Hands){
				if(card/1000==B/1000)
					return false;
			}
		}
		return true;
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		return pb_enum.BunchA;
	}

	public override bool checkDiscard(Player player,int drawCard){
		var sz=player.playData.Hands.Count;
		if(sz<=0)
			return false;
		
		return (sz%3==2);
	}

	public static void prepareAAAA(Player player){
		//check aditional AAAA
		var hands=player.playData.Hands;
		int M=3;
		List<List<int>>[] mc=new List<List<int>>[M];
		for(int i=0;i<M;++i){
			mc[i]=new List<List<int>>();
			for(int l=0;l<9;++l)mc[i].Add(new List<int>());
		}
		foreach(var B in hands){
			if(B/1000==player.playData.SelectedCard)continue;
			
			int x=B/1000-1;
			mc[x][B%100-1].Add(B);
		}
		for(int j=0; j<M; ++j){
			var v=mc[j];
			foreach(var iv in v){
				if(iv.Count==4){
					//BUNCH_AAAA
					var bunch=new bunch_t();
					bunch.Pos=player.playData.Seat;
					bunch.Type=pb_enum.BunchAaaa;
					foreach(var x in iv)bunch.Pawns.Add(x);
					player.AAAAs.Add(bunch);
				}
			}
		}
	}

	public override int comparision(int x,int y){
		var cx=(int)x/1000;
		var cy=(int)y/1000;
		if(cx<cy)return -1;
		else if(cx==cy)return (int)x%100-(int)y%100;
		else return 1;
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
			if(aiController==null)aiController=new MahjongAIController();
			return aiController;
		}
	}
	// ------------------------------------------------------
	// resources related
	// ------------------------------------------------------
	override public string CardPrefab{get{return "Mahjong";}}
	override public string Id2File(int color,int value){
		color-=1;
		string[] Colors={"tong","tiao","wan"};
		value=inverseTransformValue(value);
		if(color<Colors.Length)
			return CardPrefab+"/"+string.Format("{0}{1:0}",Colors[color],value);
		return "";
	}
	
	override public void PrepareCache(){
		var files=new List<string>();
		files.Add(CardPrefab+"/"+"dong");
		files.Add(CardPrefab+"/"+"nan");
		files.Add(CardPrefab+"/"+"xi");
		files.Add(CardPrefab+"/"+"bei");
		files.Add(CardPrefab+"/"+"zhong");
		files.Add(CardPrefab+"/"+"fa");
		files.Add(CardPrefab+"/"+"bai");
		for(int k=1;k<=3;++k)for(int i=1;i<=9;++i)
			files.Add(Id2File(k,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Mahjong"));
	}
	
	override public float DiscardScalar{get{return 1f;}}

	override public void LoadBunch(Transform parent=null,System.Action<Bunch> action=null,string path=null){
		Main.Instance.StartCoroutine(Main.Instance.updater.Load<MahjongBunch>(
			"Prefabs/MahjongBunch",parent,delegate(Object obj,Hashtable arg){
			if(action !=null){
				var zb=obj as Bunch;
				action.Invoke(zb);
			}
		}));
	}
}
