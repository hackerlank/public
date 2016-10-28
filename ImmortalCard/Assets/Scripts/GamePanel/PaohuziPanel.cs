using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	public GameObject	BtnABC,BtnA3,BtnWin;
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string CardPrefab{get{return "Zipai";}}
	override public string Id2File(int color,int value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"s","b"};
			value=Rule.inverseTransformValue(value);
			if(color<Colors.Length)
				return CardPrefab+"/"+string.Format("{0}{1:00}",Colors[color],value);
		}
		return "";
	}

	float AbandonScalar{get{return 1f;}}
	override public float DiscardScalar{get{return 1f;}}

	public override void TapCard(Card card,bool select=true){
		var selected=false;
		foreach(var old in _selection)if(old==card)selected=true;
		deselectAll();
		if(!selected)base.TapCard(card,select);
	}

	override protected void OnMsgEngage(MsgNCEngage msg){
		checkNaturalWin();
	}

	override protected void onMsgStart(){
		//transform position
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);
		StartCoroutine(sortHands());

		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}
	
	List<bunch_t> _hints=null;
	override protected void onMsgDiscard(MsgNCDiscard msg){
		//show hints for others
		if(msg.Bunch.Pawns.Count>0){
			if(Main.Instance.MainPlayer.pos!=msg.Bunch.Pos){
				var card=msg.Bunch.Pawns[0];
				if(!showHints(card,false)){
					StartCoroutine(passMeld(Main.Instance.MainPlayer,card));
					Debug.Log(Main.Instance.MainPlayer.pos+" pass after "+msg.Bunch.Pos+" discard");
				}
			}else{
				StartCoroutine(sortHands());
				Main.Instance.MainPlayer.unpairedCards.Add(msg.Bunch.Pawns[0]);
			}
		}
	}

	override protected void OnMsgDraw(int id,int pos){
		//remove discards
		foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
		//discard
		Card.Create(CardPrefab,id,Pile,delegate(Card card) {
			card.DiscardTo(DiscardAreas[pos],DiscardScalar);
			card.state=Card.State.ST_DISCARD;
		});

		//immediately pass for the drawer,we only meld when discard
		Player player=Main.Instance.MainPlayer;
		if(player.pos!=pos)foreach(var robot in Main.Instance.robots){
			if(robot.pos==pos){
				player=robot;
				break;
			}
		}
		StartCoroutine(passMeld(player,id,false));
		Debug.Log(pos+" directly pass after self draw");
	}

	override protected IEnumerator OnMsgMeld(bunch_t bunch){
		_hints=null;
		
		var from=Rule.Token;
		var to=bunch.Pos;
		var scalar=(to==_pos?DiscardScalar:AbandonScalar);
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();

		var bDraw=(bunch.Type==pb_enum.OpPass&&to!=-1);
		if(A==null&&!bDraw)
			yield break;

		switch(bunch.Type){
		case pb_enum.PhzAbc:
		case pb_enum.PhzAbA:
		case pb_enum.PhzBbb:
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
		case pb_enum.PhzAaaa:
		case pb_enum.PhzAaaastart:
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzBbbB:
		case pb_enum.PhzBbbbdesk:
			//meld,make a bunch with constant 4 cards
			List<Card> cards=new List<Card>();
			if(to==_pos){
				//find my hands cards
				var hands=HandAreas[to].GetComponentsInChildren<Card>();
				foreach(var id in bunch.Pawns){
					foreach(var card in hands){
						if(card.Value==id)
							cards.Add(card);
					}
				}
				Debug.Log("----meld prepare my bunch size="+cards.Count);
			}else{
				//add meld cards for others
				foreach(var id in bunch.Pawns){
					if(A.Value!=id) Card.Create(CardPrefab,id,MeldAreas[to],delegate(Card other){
						if(null!=other)
							cards.Add(other);
					});
				}
				while(cards.Count<bunch.Pawns.Count-1)yield return null;
				Debug.Log("----meld prepare other bunch size="+cards.Count);
			}
			cards.Add(A);

			//add place holder
			if(cards.Count<4) Card.Create(CardPrefab,-1,MeldAreas[to],delegate(Card blank){
				if(null!=blank)
					cards.Add(blank);
			});
			while(cards.Count<4)yield return null;
			Debug.Log("----meld final bunch size="+cards.Count);

			//move to meld area
			foreach(var c in cards){
				c.state=Card.State.ST_MELD;
				c.DiscardTo(MeldAreas[to],scalar);
			}
			Debug.Log("meld "+A.Value+" from "+from+" to "+to);
			
			if(to==_pos)StartCoroutine(sortHands());
			break;
		default:
			//abandon
			if(bunch.Type==pb_enum.OpPass&&-1!=to){
				//was draw
				if(to==_pos){
					var card=bunch.Pawns[0];
					MsgCNDiscard omsgDiscard=new MsgCNDiscard();
					omsgDiscard.Mid=pb_msg.MsgCnDiscard;
					omsgDiscard.Bunch=new bunch_t();
					omsgDiscard.Bunch.Pos=_pos;
					omsgDiscard.Bunch.Pawns.Add(card);
					omsgDiscard.Bunch.Type=pb_enum.BunchA;
					Main.Instance.MainPlayer.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
					Debug.Log(_pos+" discard "+card+" after self draw");
				}
			}else if(A!=null){
				if(to==-1)to=Rule.Token;
				A.DiscardTo(AbandonAreas[to],AbandonScalar);
				A.state=Card.State.ST_ABANDON;
			}
			break;
		}
		//remove from hands
		var player=Main.Instance.MainPlayer;
		if(player.pos==bunch.Pos){
			Rule.Meld(player,bunch);
		}
	}
	
	override protected bool showHints(int card,bool bDraw,bool startup=false){
		var player=Main.Instance.MainPlayer;
		var bunch=new bunch_t();
		bunch.Pos=(bDraw?player.pos:player.pos+1);
		bunch.Type=pb_enum.BunchA;
		bunch.Pawns.Add(card);

		_hints=Rule.Hint(player,bunch,true);

		//show/hide buttons
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.BunchAbc:
				if(!startup)BtnABC.SetActive(true);
				break;
			case pb_enum.PhzBbb:
				if(!startup)BtnA3.SetActive(true);
				break;
			case pb_enum.BunchAaa:
			case pb_enum.PhzAaawei:
			case pb_enum.PhzAaachou:
			case pb_enum.PhzAaaa:
				OnAAA();
				return true;
			case pb_enum.PhzAaaastart:
			case pb_enum.PhzAaaadesk:
			case pb_enum.PhzBbbB:
			case pb_enum.PhzBbbbdesk:
				OnAAAA();
				return true;
			case pb_enum.BunchWin:
				BtnWin.SetActive(true);
				break;
			default:
				break;
			}
		}
		if(_hints.Count>0)BtnPass.SetActive(true);

		return _hints.Count>0;
	}

	override protected IEnumerator sortHands(){
		var hands=HandAreas[_pos].GetComponentsInChildren<Card>();
		List<List<int>> sorted=new List<List<int>>();
		List<Card> emptyCards=new List<Card>();
		List<Card> normalCards=new List<Card>();

		//sort by value
		List<int>[] all=new List<int>[20];
		for(int i=0;i<20;++i)all[i]=new List<int>();
		foreach(var card in hands){
			var id=card.Value;
			if(id>1000){
				var j=(id/1000-1)*10+id%100-1;
				all[j].Add(id);
				normalCards.Add(card);
			}else
				emptyCards.Add(card);
		}

		//sort bunches
		for(int i=0;i<10;++i){
			var j=i+10;
			if(all[i].Count==3){
				//AAA
				sorted.Add(new List<int>(all[i]));
				all[i].Clear();
			}else if(all[j].Count==3){
				//AAA
				sorted.Add(new List<int>(all[j]));
				all[j].Clear();
			}else if(all[i].Count==2&&all[j].Count==1){
				//AAa
				all[i].Add(all[j][0]);
				sorted.Add(new List<int>(all[i]));
				all[i].Clear();
				all[j].Clear();
			}else if(all[j].Count==2&&all[i].Count==1){
				//AAa
				all[j].Add(all[i][0]);
				sorted.Add(new List<int>(all[j]));
				all[i].Clear();
				all[j].Clear();
			}else if(all[i].Count==2){
				//AA
				sorted.Add(new List<int>(all[i]));
				all[i].Clear();
			}else if(all[j].Count==2){
				//AA
				sorted.Add(new List<int>(all[j]));
				all[j].Clear();
			}
		}

		int x=0;
		var y=x+10;
		if(all[x+1].Count>0 && all[x+6].Count>0 && all[x+9].Count>0){
			//2,7,10
			var b=new List<int>();
			b.Add(all[x+1][0]);
			b.Add(all[x+6][0]);
			b.Add(all[x+9][0]);
			all[x+1].RemoveAt(0);
			all[x+6].RemoveAt(0);
			all[x+9].RemoveAt(0);
			sorted.Add(b);
		}else if(all[y+1].Count>0 && all[y+6].Count>0 && all[y+9].Count>0){
			//2,7,10
			var b=new List<int>();
			b.Add(all[y+1][0]);
			b.Add(all[y+6][0]);
			b.Add(all[y+9][0]);
			all[y+1].RemoveAt(0);
			all[y+6].RemoveAt(0);
			all[y+9].RemoveAt(0);
			sorted.Add(b);
		}else if(all[x].Count>0 && all[x+1].Count>0 && all[x+2].Count>0){
			//1,2,3
			var b=new List<int>();
			b.Add(all[x+0][0]);
			b.Add(all[x+1][0]);
			b.Add(all[x+2][0]);
			all[x+0].RemoveAt(0);
			all[x+1].RemoveAt(0);
			all[x+2].RemoveAt(0);
			sorted.Add(b);
		}else if(all[y].Count>0 && all[y+1].Count>0 && all[y+2].Count>0){
			//1,2,3
			var b=new List<int>();
			b.Add(all[y+0][0]);
			b.Add(all[y+1][0]);
			b.Add(all[y+2][0]);
			all[y+0].RemoveAt(0);
			all[y+1].RemoveAt(0);
			all[y+2].RemoveAt(0);
			sorted.Add(b);
		}

		//deliver the rest by average
		List<int> rest=new List<int>();
		for(int i=0;i<20;++i)if(all[i].Count>0)rest.Add(all[i][0]);

		int iRest=0;
		//emptyColume could not be 0: 20/2=10; cardsPerColume as well
		int emptyColume=11-sorted.Count;
		int columes=Mathf.Min(rest.Count,emptyColume);
		if(columes>0){
			int cardsPerColume=rest.Count/columes;
			for(int i=0;i<columes;++i){
				var b=new List<int>();
				int J=cardsPerColume;
				if(i==0)J+=rest.Count-cardsPerColume*columes;
				for(int j=0;j<J;++j)b.Add(rest[iRest++]);
				sorted.Add(b);
			}
		}
		//insert blank if need
		foreach(var s in sorted)
			for(int i=s.Count;i<3;++i)
				s.Insert(0,-1);

		//add or remove empty cards
		int wait=sorted.Count*3-hands.Length;
		for(int i=hands.Length;i<sorted.Count*3;++i){
			Card.Create(CardPrefab,-1,HandAreas[_pos],delegate(Card obj) {
				emptyCards.Add(obj);
				wait--;
			});
		}
		while(wait>0)yield return null;

		for(int i=sorted.Count*3;i<hands.Length;++i){
			Destroy(emptyCards[0].gameObject);
			emptyCards.RemoveAt(0);
		}

		int iempty=0;
		int index=0;
		foreach(var bun in sorted){
			foreach(var id in bun){
				if(id<1000)
					emptyCards[iempty++].transform.SetSiblingIndex(index++);
				else foreach(var card in normalCards){
					if(card.Value==id){
						card.transform.SetSiblingIndex(index++);
						break;
					}
				}
			}
		}
	}
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new PaohuziRule();

		var files=new List<string>();
		files.Add(CardPrefab+"/"+"back");
		for(int k=1;k<=2;++k)for(int i=1;i<=10;++i)
			files.Add(Id2File(k,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Zipai"));

		btnOps=new GameObject[]{BtnABC,BtnA3,BtnWin,BtnPass};
	}

	public void OnABC(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints!=null&&_hints.Count>0){
			//TODO: show selection for multiple hints
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.PhzAbc||
					hint.Type==pb_enum.PhzAbA){
					MsgCNMeld msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=new bunch_t();
					msg.Bunch.Pos=_pos;
					msg.Bunch.Type=hint.Type;
					msg.Bunch.MergeFrom(hint);
					Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
					break;
				}
			}
		}
	}
	
	public void OnAAA(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints!=null&&_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.PhzAaa ||
				   hint.Type==pb_enum.PhzAaawei ||
				   hint.Type==pb_enum.PhzAaachou ||
				   hint.Type==pb_enum.PhzBbb){
					MsgCNMeld msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=new bunch_t();
					msg.Bunch.Pos=_pos;
					msg.Bunch.Type=hint.Type;
					msg.Bunch.MergeFrom(hint);
					Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
					break;
				}
			}
		}
	}
	
	public void OnAAAA(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints!=null&&_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.PhzAaaa ||
				   hint.Type==pb_enum.PhzAaaadesk ||
				   hint.Type==pb_enum.PhzAaaastart ||
				   hint.Type==pb_enum.PhzBbbB ||
				   hint.Type==pb_enum.PhzBbbbdesk){
					MsgCNMeld msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=new bunch_t();
					msg.Bunch.Pos=_pos;
					msg.Bunch.Type=hint.Type;
					msg.Bunch.MergeFrom(hint);
					Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
					break;
				}
			}
		}
	}
	
	public void OnWin(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints!=null&&_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.BunchWin){
					MsgCNMeld msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=new bunch_t();
					msg.Bunch.Pos=_pos;
					msg.Bunch.Type=hint.Type;
					msg.Bunch.MergeFrom(hint);
					Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
					break;
				}
			}
		}
	}
	
	public static void Create(System.Action<Component> handler=null){
		Utils.Load<PaohuziPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
