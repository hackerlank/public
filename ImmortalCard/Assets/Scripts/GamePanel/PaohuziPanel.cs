using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziPanel : GamePanel {
	public GameObject	BtnABC,BtnA3,BtnA4,BtnWin;
	public GameObject	BaihuoPanel;
	public Transform[]	BaihuoLayers;	//max 3 layers

	List<bunch_t> _hints=new List<bunch_t>();

	override public IEnumerator OnMsgDeal(Player player,MsgNCDeal msg){
		//deal and random display all hands include AAAA and AAA
		yield return StartCoroutine(base.OnMsgDeal(player,msg));
		//transform position
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);

		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}
	
	override public IEnumerator OnMsgRevive(Player player,MsgNCRevive msg){
		yield return StartCoroutine(base.OnMsgRevive(player,msg));
		
		if(msg.Result!=pb_enum.Succeess)
			yield break;
		
		PaohuziRule.prepareAAAA(player);

		for(int i=0;i<maxPlayer;++i){
			if(i<MeldAreas.Length)
				foreach(Transform ch in MeldAreas[i])Destroy(ch.gameObject);
			if(i<AbandonAreas.Length)
				foreach(Transform ch in AbandonAreas[i])Destroy(ch.gameObject);
			
			var playFrom=msg.Play[i];
			
			//revive meld
			foreach(var meld in playFrom.Bunch){
				Bunch mjBunch=null;
				Rule.LoadBunch(MeldAreas[i],delegate(Bunch zb) {
					mjBunch=zb;
					mjBunch.Value=meld;
				});
				while(!mjBunch)yield return null;
			}
			
			//revive abandon
			foreach(var id in playFrom.Discards){
				var fin=false;
				Card.Create(Rule.CardPrefab,id,AbandonAreas[i],delegate(Card card) {
					card.state=Card.State.ST_ABANDON;
					fin=true;
				});
				while(!fin)yield return null;
			}
		}
		yield return StartCoroutine(sortHands());
	}

	bool preEngage=false;
	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		while(!preEngage)yield return null;
		yield return StartCoroutine(sortHands());
		checkNaturalWin();
		preEngage=false;
	}
	
	override public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		//should execute immediately before remove card from hands
		StartCoroutine(base.OnMsgDiscard(player,msg));

		if(Main.Instance.MainPlayer!=player)yield break;
		if(msg.Bunch.Pawns.Count<=0){
			Debug.LogError("error when OnMsgDiscard");
			yield break;
		}

		//show hints
		var fromMyHands=false;
		var card=msg.Bunch.Pawns[0];
		Zipai.PlaySound(card);
		foreach(var h in player.playData.Hands)if(h==card){fromMyHands=true;break;}
		if(fromMyHands){
			//StartCoroutine(sortHands());
			Main.Instance.MainPlayer.unpairedCards.Add(card);
		}else{
			if(!showHints(msg.Bunch)){
				StartCoroutine(passMeld(Main.Instance.MainPlayer,card));
				//Debug.Log(Main.Instance.MainPlayer.playData.Seat+" pass after "+msg.Bunch.Pos+" discard");
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
		Card.Create(Rule.CardPrefab,1000,Pile,delegate(Card obj) {
			card=obj;
		});
		while(card==null)yield return null;

		card.DiscardTo(DiscardAreas[pos],Rule.DiscardScalar);
		card.state=Card.State.ST_DISCARD;

		Debug.Log(pos+" draw "+id);
		yield return new WaitForSeconds(Config.OpsInterval/2f);

		//immediately discard,we only meld when discard
		if(this!=null){
			card.Value=id;
			
			if(player.playData.Seat!=pos)foreach(var robot in Main.Instance.robots){
				if(robot.playData.Seat==pos){
					player=robot;
					break;
				}
			}
			var omsg=new MsgCNDiscard();
			omsg.Mid=pb_msg.MsgCnDiscard;
			bunch_t bunch=new bunch_t();
			bunch.Pos=player.playData.Seat;
			bunch.Pawns.Add(id);
			bunch.Type=pb_enum.BunchA;
			omsg.Bunch=bunch;
			player.Send<MsgCNDiscard>(omsg.Mid,omsg);
		}
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		var bunch=msg.Bunch;
		var from=msg.From;
		var to=bunch.Pos;
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();

		var bDraw=Rule.Pile.IndexOf(bunch.Pawns[0])!=-1;
		if(A==null&&!bDraw)
			yield break;

		ZipaiBunch.PlaySound(bunch.Type);
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
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
			//remove bunch from desk
			var meldBunch=MeldAreas[to].GetComponentsInChildren<Bunch>();
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
			if(bunch.Pos!=msg.From){
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
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
		case pb_enum.PhzAbc:
		case pb_enum.PhzBbb:
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
		case pb_enum.PhzAaaa:
		case pb_enum.PhzBbbB:
			//move cards from hands and discard to meld area
			foreach(var melt in melds){
				if(to==_pos){
					//find my hands cards and remove
					var hands=HandAreas[to].GetComponentsInChildren<Card>();
					foreach(var id in melt.Pawns){
						foreach(var h in hands){
							if(h.Value==id){
								h.RemoveFromHand();
								break;
							}
						}
					}
				}

				//meld bunch
				Bunch zb=null;
				Rule.LoadBunch(MeldAreas[to],delegate(Bunch obj) {
					zb=obj;
					zb.Value=melt;
				});
				while(zb==null)yield return null;

				//Debug.Log("meld "+A.Value+" from "+from+" to "+to);
			}
			Destroy(A.gameObject);

			break;
		case pb_enum.OpPass:
			//abandon
			if(A!=null){
				A.DiscardTo(AbandonAreas[from],AbandonScalar);
				A.state=Card.State.ST_ABANDON;
			}
			break;
		default:
			break;
		}
		//handle pass and dodge
		switch(bunch.Type){
		case pb_enum.PhzAbc:
		case pb_enum.OpPass:
			//remember past and dodge cards
			var card=bunch.Pawns[0];
			var me=Main.Instance.MainPlayer;
			//by the hints in children
			var past=false;
			var dodge=false;
			foreach(var hint in bunch.Child){
				if(me.playData.Seat==hint.Pos){
					if(hint.Type==pb_enum.PhzBbb)
						dodge=true;
					if(hint.Type==pb_enum.PhzAbc)
						past=true;
				}
			}
			if(past && bunch.Type==pb_enum.PhzAbc){
				//if other player prior to me,shouldn't make me pass
				var X=from;
				var Y=_pos;
				var Z=to;
				if(X==maxPlayer)X=0;
				else{
					if(Y==0)Y=maxPlayer;
					if(Z==0)Z=maxPlayer;
				}
				if(Z-X<Y-X)past=false;
			}

			if(dodge)me.dodgeCards.Add(card);
			else if(past)me.unpairedCards.Add(card);
			break;
		}
		//remove from hands
		if(player.playData.Seat==bunch.Pos){
			Rule.Meld(player,bunch);
		}
		//if(to==_pos)StartCoroutine(sortHands());

		yield return StartCoroutine(base.OnMsgMeld(player,msg));
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));

		for(int i=0;i<MeldAreas.Length;++i)foreach(Transform ch in MeldAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<AbandonAreas.Length;++i)foreach(Transform ch in AbandonAreas[i].transform)Destroy(ch.gameObject);
		StartCoroutine(Main.Instance.updater.Load<PaohuziSettle>(
			"Prefabs/PaohuziSettle",Main.Instance.RootPanel,delegate(Object obj,Hashtable arg) {
			var popup=obj as SettlePopup;
			popup.Value=msg;
		}));
	}

	
	override public IEnumerator PostMessage(pb_msg mid,byte[] bytes){
		switch(mid){
		case pb_msg.MsgNcBeforeStartup:
			MsgNcBeforeStartup msgBeforeStartup=MsgNcBeforeStartup.Parser.ParseFrom(bytes);
			//process all players AAAA
			PaohuziRule.prepareAAAA(Main.Instance.MainPlayer);
			
			for(int i=0;i<maxPlayer;++i){
				bunch_t bunch=msgBeforeStartup.Bunch[i];
				for(int j=0;j<bunch.Pawns.Count/4;++j){
					var val=new bunch_t();
					for(int k=j*4;k<(j+1)*4;++k)val.Pawns.Add(bunch.Pawns[k]);
					
					//remove cards from hand area
					if(_pos==i){
						var hands=HandAreas[i].GetComponentsInChildren<Card>();
						foreach(var pawn in bunch.Pawns){
							foreach(var hand in hands){
								if(hand.Value==pawn){
									Destroy(hand.gameObject);
									break;
								}
							}
						}
						//make destroy effective
						yield return null;
					}
					
					//meld area
					Bunch zb=null;
					Rule.LoadBunch(MeldAreas[i],delegate(Bunch obj) {
						zb=obj;
						zb.Value=val;
					});
					while(zb==null)yield return null;
				}
			}
			preEngage=true;
			break;
		default:
			break;
		}
	}

	override protected IEnumerator deal(MsgNCDeal msg){
		var hands=new List<int>(msg.Hands);
		//hands.Sort(Rule.comparision);
		
		//deal
		string str="deal: banker="+msg.Banker+",pos="+msg.Pos+",hands:\n";
		int rest=hands.Count%3;
		int col=hands.Count/3+(rest==0?0:1);
		for(int i=0;i<col;++i){
			ZipaiHandBunch bunch=null;
			yield return StartCoroutine(Main.Instance.updater.Load<ZipaiHandBunch>(
				"Prefabs/ZipaiHandBunch",HandAreas[0],delegate(Object obj,Hashtable arg){
				bunch=obj as ZipaiHandBunch;
			}));

			for(int j=0;j<3;++j){
				var idx=i*3+j;
				if(idx<hands.Count){
					var id=hands[idx];
					bunch.Add(id);
					str+=id.ToString()+",";
					if((idx+1)%6==0)str+="\n";
				}
			}
		}
		Debug.Log(str);
	}

	override protected IEnumerator passMeld(Player player,int card=0,bool wait=true){
		//send past and dodge operation,remember when message back
		player.unpairedCards.Remove(card);
		var past=false;
		var dodge=false;
		foreach(var b in _hints){
			if(b.Type==pb_enum.PhzAbc){
				past=true;
				Debug.Log(player.playData.Seat+" pass "+card);
			}else if(b.Type==pb_enum.PhzBbb){
				dodge=true;
				Debug.Log(player.playData.Seat+" dodge "+card);
			}
		}

		if(wait)yield return new WaitForSeconds(Config.OpsInterval);
		
		//pass discard or draw message
		var msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		
		bunch_t bunch=new bunch_t();
		bunch.Pos=player.playData.Seat;
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

		_hints=Rule.Hint(player,bunch);

		//show/hide buttons
		var abc=false;
		var bbb=false;
		var bbbb=false;
		var win=false;
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.PhzAbc:
				abc=true;
				break;
			case pb_enum.PhzBbb:
				bbb=true;
				break;
			case pb_enum.PhzAaa:
			case pb_enum.PhzAaawei:
			case pb_enum.PhzAaachou:
				//auto meld
				OnAAA();
				return true;
			case pb_enum.PhzAaaa:
			case pb_enum.PhzAaaastart:
			case pb_enum.PhzAaaadesk:
				//auto meld
				OnAAAA();
				return true;
			case pb_enum.PhzBbbB:
			case pb_enum.PhzB4B3:
			case pb_enum.PhzBbbbdesk:
				bbbb=true;
				break;
			case pb_enum.BunchWin:
				win=true;
				break;
			case pb_enum.OpPass:
				BtnPass.SetActive(true);
				break;
			default:
				break;
			}
		}
		if(win)
			BtnWin.SetActive(true);
		else if(bbbb){
			//auto meld
			OnAAAA();
			return true;
		}

		var forceWin=false;
		if(!startup){
			if(abc)BtnABC.SetActive(true);
			if(bbb)BtnA3.SetActive(true);
			if(bbbb)BtnA4.SetActive(true);
		}else if(win){
			//force win 3 AAAA
			foreach(var b in _hints){
				if(b.Type==pb_enum.BunchWin){
					var A4=0;
					foreach(var hint in b.Child){
						if(hint.Type==pb_enum.PhzAaaastart)
							++A4;
					}
					if(A4>=3){
						forceWin=true;
						break;
					}
				}
			}
		}

		var ret=(win||
		   !startup && (bbbb||bbb||abc));
		BtnPass.SetActive(ret && !forceWin);
		return ret;
	}

	override protected IEnumerator sortHands(){
		var player=Main.Instance.MainPlayer;
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
				//sorted.Add(new List<int>(all[i]));
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
				//sorted.Add(new List<int>(all[j]));
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
		int emptyColume=11-sorted.Count-player.AAAs.Count;
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

		foreach(Transform trans in HandAreas[0])Destroy(trans.gameObject);
		yield return null;

		foreach(var aaa in player.AAAs){
			ZipaiHandBunch bunch=null;
			yield return StartCoroutine(Main.Instance.updater.Load<ZipaiHandBunch>(
				"Prefabs/ZipaiHandBunch",HandAreas[0],delegate(Object obj,Hashtable arg){
				bunch=obj as ZipaiHandBunch;
			}));

			foreach(var id in aaa.Pawns)bunch.Add(id,true);
		}
		foreach(var cards in sorted){
			ZipaiHandBunch bunch=null;
			yield return StartCoroutine(Main.Instance.updater.Load<ZipaiHandBunch>(
				"Prefabs/ZipaiHandBunch",HandAreas[0],delegate(Object obj,Hashtable arg){
				bunch=obj as ZipaiHandBunch;
			}));
			foreach(var id in cards)bunch.Add(id);
		}
	}
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new PaohuziRule();

		btnOps=new GameObject[]{BtnABC,BtnA3,BtnA4,BtnWin,BtnPass};
	}

	public void OnABC(){
		if(_hints.Count>0)
			StartCoroutine(onABC());
	}

	bool onTap(bunch_t[] bunchLayers,Bunch zpbunch){
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
			Bunch zipaiBunch=null;
			Rule.LoadBunch(BaihuoLayers[layer],delegate(Bunch obj) {
				zipaiBunch=obj;
				zipaiBunch.Value=bunch;
				zipaiBunch.onTap=delegate(Bunch obj1) {
					_baihuoReady=onTap(bunchLayers,obj1);
				};
			});
			while(zipaiBunch==null)yield return null;
		}
	}

	bool _baihuoReady=true;
	IEnumerator onABC(){
		foreach(var btn in btnOps)btn.SetActive(false);
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
		cancelABC();

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
		_hints.Clear();
	}
	
	public void OnAAA(){
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
		foreach(var btn in btnOps)btn.SetActive(false);
		_hints.Clear();
	}
	
	public void OnAAAA(){
		if(_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.PhzAaaa ||
				   hint.Type==pb_enum.PhzAaaadesk ||
				   hint.Type==pb_enum.PhzAaaastart ||
				   hint.Type==pb_enum.PhzBbbB ||
				   hint.Type==pb_enum.PhzB4B3 ||
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
		foreach(var btn in btnOps)btn.SetActive(false);
		_hints.Clear();
	}
	
	public void OnWin(){
		if(_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.BunchWin){
					MsgCNMeld msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=new bunch_t();
					msg.Bunch.MergeFrom(hint);

					string str="when win:";
					str+=Player.bunch2str(msg.Bunch);
					if(msg.Bunch.Child.Count>0){
						str+="{";
						foreach(var ch in msg.Bunch.Child)str+=Player.bunch2str(ch);
						str+="}\n";
					}
					Debug.Log(str);

					Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
					break;
				}
			}
		}
		foreach(var btn in btnOps)btn.SetActive(false);
		_hints.Clear();
	}

	public void OnCancelABC(){
		OnPass();
		cancelABC();
		foreach(var btn in btnOps)btn.SetActive(false);
		_hints.Clear();
	}

	void cancelABC(){
		BaihuoPanel.SetActive(false);
		foreach(var panel in BaihuoLayers)
			foreach(Transform trans in panel)
				Destroy(trans.gameObject);
	}
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	float AbandonScalar{get{return 1f;}}

	public override void TapCard(Card card,bool select=true){
		var selected=false;
		foreach(var old in _selection)if(old==card)selected=true;
		deselectAll();
		if(!selected)base.TapCard(card,select);
	}

	public static void Create(System.Action<Component> handler=null){
		Main.Instance.StartCoroutine(Main.Instance.updater.Load<PaohuziPanel>(
			"Prefabs/PaohuziPanel",Main.Instance.RootPanel,delegate(Object obj,Hashtable arg){
			if(handler!=null)handler.Invoke(obj as Component);
		}));
	}
}
