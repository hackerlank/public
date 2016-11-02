using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	public GameObject	BtnABC,BtnA3,BtnWin;
	public GameObject	BaihuoPanel;
	public Transform[]	BaihuoLayers;	//max 3 layers

	List<bunch_t> _hints=new List<bunch_t>();

	override public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		yield return StartCoroutine(base.OnMsgStart(player,msg));
		//transform position
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);

		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}
	
	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		PaohuziRule.prepareAAAA(Main.Instance.MainPlayer);
		//display all AAAA
		for(int i=0;i<maxPlayer;++i){
			if(i==_pos)continue;
			
			var bunch=msg.Bunch[i];
			ZipaiBunch zb=null;
			Utils.Load<ZipaiBunch>(MeldAreas[i],delegate(Component obj) {
				zb=obj as ZipaiBunch;
			});
			while(zb==null)yield return null;
			zb.Value=bunch;
		}
		
		//remove AAAA from hands
		var hands=HandAreas[_pos].GetComponentsInChildren<Card>();
		foreach(var id in msg.Bunch[_pos].Pawns){
			foreach(var card in hands){
				if(card.Value==id)
					card.DiscardTo(MeldAreas[_pos],DiscardScalar);
			}
		}
		
		yield return StartCoroutine(sortHands());
		checkNaturalWin();
	}
	
	override public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		//should execute immediately before remove card from hands
		StartCoroutine(base.OnMsgDiscard(player,msg));

		if(Main.Instance.MainPlayer!=player)yield break;
		//show hints
		var fromSelf=false;
		var card=msg.Bunch.Pawns[0];
		foreach(var h in player.playData.Hands)if(h==card){fromSelf=true;break;}
		Debug.Log("-- check hands when discard, fromSelf="+fromSelf+",card="+card);
		if(fromSelf){
			StartCoroutine(sortHands());
			Main.Instance.MainPlayer.unpairedCards.Add(msg.Bunch.Pawns[0]);
		}else{
			if(!showHints(msg.Bunch)){
				StartCoroutine(passMeld(Main.Instance.MainPlayer,card));
				Debug.Log(Main.Instance.MainPlayer.pos+" pass after "+msg.Bunch.Pos+" discard");
			}
		}
	}
	
	override public IEnumerator OnMsgDraw(Player player,MsgNCDraw msg){
		yield return StartCoroutine(base.OnMsgDraw(player,msg));

		var id=msg.Card;
		var pos=msg.Pos;
		//remove discards
		foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
		//discard
		Card card=null;
		Card.Create(CardPrefab,1000,Pile,delegate(Card obj) {
			card=obj;
		});
		while(card==null)yield return null;

		card.DiscardTo(DiscardAreas[pos],DiscardScalar);
		card.state=Card.State.ST_DISCARD;

		yield return new WaitForSeconds(Configs.OpsInterval/2f);
		card.Value=id;

		//immediately pass for the drawer,we only meld when discard
		if(player.pos!=pos)foreach(var robot in Main.Instance.robots){
			if(robot.pos==pos){
				player=robot;
				break;
			}
		}
		StartCoroutine(passMeld(player,id,false));
		Debug.Log(pos+" directly pass after self draw");
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		_hints.Clear();

		var bunch=msg.Bunch;
		var from=Rule.Token;
		var to=bunch.Pos;
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();

		var bDraw=(bunch.Type==pb_enum.OpPass&&to!=-1);
		if(A==null&&!bDraw)
			yield break;

		//construct bunches
		var melds=new List<bunch_t>();
		switch(bunch.Type){
		case pb_enum.PhzAbc:
			for(var i=0;i<bunch.Pawns.Count/3;++i){
				var bun=new bunch_t();
				bun.Pos=bunch.Pos;
				bun.Type=bunch.Type;
				bun.Pawns.Add(bunch.Pawns[i*3+0]);
				bun.Pawns.Add(bunch.Pawns[i*3+1]);
				bun.Pawns.Add(bunch.Pawns[i*3+2]);
				melds.Add(bun);
			}
			break;
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzBbbbdesk:
			//remove bunch from desk
			var meldBunch=MeldAreas[to].GetComponentsInChildren<ZipaiBunch>();
			foreach(var mb in meldBunch){
				var found=false;
				var val=mb.Value;
				foreach(var b in val.Pawns){
					foreach(var a in bunch.Pawns){
						if(a==b){
							found=true;
							break;
						}
					}
					if(found)break;
				}
				if(found){
					Destroy(mb.gameObject);
					break;
				}
			}
			melds.Add(bunch);

			//mark conflict meld
			if(bunch.Pos!=Rule.Token){
				//Rule.Token conflict
			}
			break;
		default:
			melds.Add(bunch);
			break;
		}

		//then move cards
		switch(bunch.Type){
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzBbbbdesk:
		case pb_enum.PhzAbc:
		case pb_enum.PhzBbb:
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
		case pb_enum.PhzAaaa:
		case pb_enum.PhzBbbB:
			//move cards from hands and discard to meld area
			foreach(var melt in melds){
				ZipaiBunch zb=null;
				Utils.Load<ZipaiBunch>(MeldAreas[to],delegate(Component obj) {
					zb=obj as ZipaiBunch;
				});
				while(zb==null)yield return null;
				zb.Value=melt;

				if(to==_pos){
					//find my hands cards
					var hands=HandAreas[to].GetComponentsInChildren<Card>();
					foreach(var id in melt.Pawns){
						foreach(var h in hands){
							if(h.Value==id){
								Destroy(h.gameObject);
								break;
							}
						}
					}
				}

				Debug.Log("meld "+A.Value+" from "+from+" to "+to);
			}
			Destroy(A.gameObject);

			break;
		default:
			//abandon
			var card=bunch.Pawns[0];
			if(bunch.Type==pb_enum.OpPass&&-1!=to){
				//was draw
				if(to==_pos){
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

			//record past and dodge cards
			var me=Main.Instance.MainPlayer;
			if(me.pos==bunch.Pos){
				var past=false;
				var dodge=false;
				foreach(var b in bunch.Child){
					if(b.Type==pb_enum.PhzAbc){
						past=true;
					}else if(b.Type==pb_enum.PhzBbb){
						dodge=true;
					}
				}
				if(past)me.unpairedCards.Add(card);
				if(dodge)me.dodgeCards.Add(card);
			}
			break;
		}
		//remove from hands
		if(player.pos==bunch.Pos){
			Rule.Meld(player,bunch);
		}
		if(to==_pos)StartCoroutine(sortHands());

		yield return StartCoroutine(base.OnMsgMeld(player,msg));
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));

		for(int i=0;i<MeldAreas.Length;++i)foreach(Transform ch in MeldAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<AbandonAreas.Length;++i)foreach(Transform ch in AbandonAreas[i].transform)Destroy(ch.gameObject);
		Utils.Load<PaohuziSettle>(Main.Instance.transform,delegate(Component obj) {
			var popup=obj as SettlePopup;
			popup.Value=msg;
		});
	}

	override protected IEnumerator passMeld(Player player,int card=0,bool wait=true){
		//send past and dodge operation,record when message back
		player.unpairedCards.Remove(card);
		var past=false;
		var dodge=false;
		foreach(var b in _hints){
			if(b.Type==pb_enum.PhzAbc){
				past=true;
				Debug.Log(player.pos+" past "+card);
			}else if(b.Type==pb_enum.PhzBbb){
				dodge=true;
				Debug.Log(player.pos+" dodge "+card);
			}
		}

		if(wait)yield return new WaitForSeconds(Configs.OpsInterval);
		
		//pass discard or draw message
		var msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		
		bunch_t bunch=new bunch_t();
		bunch.Pos=player.pos;
		bunch.Pawns.Add(card);
		bunch.Type=pb_enum.OpPass;
		if(past){
			bunch_t bpast=new bunch_t();
			bpast.Type=pb_enum.PhzAbc;
			bunch.Child.Add(bpast);
		}
		if(dodge){
			bunch_t bpast=new bunch_t();
			bpast.Type=pb_enum.PhzBbb;
			bunch.Child.Add(bpast);
		}
		msg.Bunch=bunch;
		
		player.Send<MsgCNMeld>(msg.Mid,msg);
	}

	override protected bool showHints(bunch_t bunch,bool startup=false){
		var player=Main.Instance.MainPlayer;

		_hints=Rule.Hint(player,bunch,true);

		//show/hide buttons
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.PhzAbc:
				if(!startup)BtnABC.SetActive(true);
				break;
			case pb_enum.PhzBbb:
				if(!startup)BtnA3.SetActive(true);
				break;
			case pb_enum.PhzAaa:
			case pb_enum.PhzAaawei:
			case pb_enum.PhzAaachou:
				OnAAA();
				return true;
			case pb_enum.PhzAaaa:
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
			}else if(all[i].Count==2&&all[j].Count==1){
				//AAa
				all[i].Add(all[j][0]);
				sorted.Add(new List<int>(all[i]));
				all[i].Clear();
				all[j].Clear();
			}else if(all[i].Count==2){
				//AA
				sorted.Add(new List<int>(all[i]));
				all[i].Clear();
			}

			if(all[j].Count==3){
				//AAA
				sorted.Add(new List<int>(all[j]));
				all[j].Clear();
			}else if(all[j].Count==2&&all[i].Count==1){
				//AAa
				all[j].Add(all[i][0]);
				sorted.Add(new List<int>(all[j]));
				all[i].Clear();
				all[j].Clear();
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
		}
		if(all[y+1].Count>0 && all[y+6].Count>0 && all[y+9].Count>0){
			//2,7,10
			var b=new List<int>();
			b.Add(all[y+1][0]);
			b.Add(all[y+6][0]);
			b.Add(all[y+9][0]);
			all[y+1].RemoveAt(0);
			all[y+6].RemoveAt(0);
			all[y+9].RemoveAt(0);
			sorted.Add(b);
		}
		if(all[x].Count>0 && all[x+1].Count>0 && all[x+2].Count>0){
			//1,2,3
			var b=new List<int>();
			b.Add(all[x+0][0]);
			b.Add(all[x+1][0]);
			b.Add(all[x+2][0]);
			all[x+0].RemoveAt(0);
			all[x+1].RemoveAt(0);
			all[x+2].RemoveAt(0);
			sorted.Add(b);
		}
		if(all[y].Count>0 && all[y+1].Count>0 && all[y+2].Count>0){
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
				if(i<rest.Count%columes)++J;
				for(int j=0;j<J;++j)b.Add(rest[iRest++]);
				sorted.Add(b);
			}
		}

		//insert blank if need
		foreach(var s in sorted)
			for(int i=s.Count;i<3;++i)
				s.Insert(0,-1);

		//add more empty cards
		int wait=sorted.Count*3-hands.Length;
		for(int i=hands.Length;i<sorted.Count*3;++i){
			Card.Create(CardPrefab,-1,HandAreas[_pos],delegate(Card obj) {
				emptyCards.Add(obj);
				wait--;
			});
		}
		while(wait>0)yield return null;

		//or remove extra empty cards
		for(int i=sorted.Count*3;i<hands.Length;++i){
			Destroy(emptyCards[0].gameObject);
			emptyCards.RemoveAt(0);
		}

		int iempty=0;
		int index=0;
		foreach(var bun in sorted){
			foreach(var id in bun){
				if(id<1000){
					emptyCards[iempty++].transform.SetSiblingIndex(index++);
				}else foreach(var card in normalCards){
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
		if(_hints.Count>0){
			StartCoroutine(onABC());
		}
	}

	bool onTap(bunch_t[] bunchLayers,ZipaiBunch zpbunch){
		int layer=0;
		if(zpbunch.transform.parent==BaihuoLayers[2]){
			layer=2;
			bunchLayers[2]=zpbunch.Value;
		}else if(zpbunch.transform.parent==BaihuoLayers[1]){
			layer=1;
			bunchLayers[2]=null;
			bunchLayers[1]=zpbunch.Value;
		}else{
			layer=0;
			bunchLayers[2]=null;
			bunchLayers[1]=null;
			bunchLayers[0]=zpbunch.Value;
		}
		
		//expand children
		var child=zpbunch.Value.Child;
		if(child.Count>0){
			var children=new List<bunch_t>(child);
			StartCoroutine(expandSelectPanel(bunchLayers,children,layer+1));
			return false;
		}else
			return true;
	}

	IEnumerator expandSelectPanel(bunch_t[] bunchLayers,List<bunch_t> list,int layer){
		var abcs=new List<bunch_t>(); 
		for(int i=0;i<list.Count;++i){
			var bunch=list[i];
			if(bunch.Type!=pb_enum.PhzAbc)
				continue;
			
			abcs.Add(bunch);
			ZipaiBunch zipaiBunch=null;
			Utils.Load<ZipaiBunch>(BaihuoLayers[layer],delegate(Component obj) {
				zipaiBunch=obj as ZipaiBunch;
			});
			while(zipaiBunch==null)yield return null;
			zipaiBunch.Value=bunch;
			zipaiBunch.onTap=delegate(ZipaiBunch obj) {
				_baihuoReady=onTap(bunchLayers,obj);
			};
		}
	}

	bool _baihuoReady=true;
	IEnumerator onABC(){
		bunch_t[] bunchLayers=new bunch_t[3];

		var abcs=new List<bunch_t>(); 
		for(int i=0;i<_hints.Count;++i){
			var bunch=_hints[i];
			if(bunch.Type==pb_enum.PhzAbc)
				abcs.Add(bunch);
		}
		_baihuoReady=false;
		yield return StartCoroutine(expandSelectPanel(bunchLayers,abcs,0));
		BaihuoPanel.SetActive(true);

		while(!_baihuoReady)yield return null;
		BaihuoPanel.SetActive(false);

		bunch_t obunch=null;
		foreach(var bl in bunchLayers){
			if(bl==null)break;
			if(null==obunch)
				obunch=bl;
			else
				foreach(var p in bl.Pawns)obunch.Pawns.Add(p);
		}

		if(obunch!=null){
			MsgCNMeld msg=new MsgCNMeld();
			msg.Mid=pb_msg.MsgCnMeld;
			msg.Bunch=new bunch_t();
			msg.Bunch.Pos=_pos;
			msg.Bunch.Type=obunch.Type;
			msg.Bunch.MergeFrom(obunch);
			Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
		}
		yield break;
	}
	
	public void OnAAA(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints.Count>0){
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
		if(_hints.Count>0){
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
		if(_hints.Count>0){
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

	public void OnCancelABC(){
		BaihuoPanel.SetActive(false);
		OnPass();
	}
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

	public static void Create(System.Action<Component> handler=null){
		Utils.Load<PaohuziPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
