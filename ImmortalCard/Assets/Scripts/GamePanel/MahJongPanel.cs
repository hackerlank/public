using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	public GameObject	BtnA3,BtnA4,BtnWin;
	public GameObject	BtnTong,BtnTiao,BtnWan;

	override public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		yield return StartCoroutine(base.OnMsgStart(player,msg));
		//transform position
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);
	}

	override public IEnumerator OnMsgRevive(Player player,MsgNCRevive msg){
		yield return StartCoroutine(base.OnMsgRevive(player,msg));

		var btns=new GameObject[]{BtnTong,BtnTiao,BtnWan};
		foreach(var btn in btns)btn.SetActive(false);

		var pos=player.playData.Seat;
		var play=msg.Play[pos];
		//revive meld
		foreach(var meld in play.Bunch){
			MahjongBunch mjBunch=null;
			Utils.Load<MahjongBunch>(MeldAreas[pos],delegate(Component obj){
				mjBunch=obj as MahjongBunch;
				mjBunch.transform.SetSiblingIndex(0);
			});
			while(!mjBunch)yield return null;
			mjBunch.Value=meld;
		}

		//revive abandon
		foreach(var id in play.Discards){
			Card.Create(CardPrefab,id,AbandonAreas[pos],delegate(Card card) {
				card.state=Card.State.ST_ABANDON;
			});
		}
	}

	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		GameObject[] btns=new GameObject[]{BtnTong,BtnTiao,BtnWan};
		foreach(var btn in btns)btn.SetActive(false);
		
		for(int i=0;i<msg.Keys.Count;++i)
			if(Main.Instance.MainPlayer.playData.Seat==i)
				Main.Instance.MainPlayer.playData.SelectedCard=msg.Keys[i];
		
		MahJongRule.prepareAAAA(player);
		checkNaturalWin();
		yield break;
	}

	List<bunch_t> _hints=null;
	override public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		yield return StartCoroutine(base.OnMsgDiscard(player,msg));
		if(msg.Bunch.Pawns.Count<=0){
			Debug.LogError("error when OnMsgDiscard");
			yield break;
		}

		//show hints for others
		if(msg.Bunch.Pawns.Count>0&&Main.Instance.MainPlayer.playData.Seat!=msg.Bunch.Pos){
			var card=msg.Bunch.Pawns[0];
			if(!showHints(msg.Bunch)){
				StartCoroutine(passMeld(Main.Instance.MainPlayer,card));
				Debug.Log(Main.Instance.MainPlayer.playData.Seat+" pass after "+msg.Bunch.Pos+" discard");
			}
		}
	}

	override public IEnumerator OnMsgDraw(Player player,MsgNCDraw msg){
		yield return StartCoroutine(base.OnMsgDraw(player,msg));

		var pos=msg.Pos;
		var id=(pos==_pos?msg.Card:-1);
		//remove discards
		foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
		//discard
		Card.Create(CardPrefab,id,Pile,delegate(Card card) {
			card.DiscardTo(DiscardAreas[pos],DiscardScalar);
			card.state=Card.State.ST_DISCARD;
		});

		//show hints only for MainPlayer
		bunch_t bunch=new bunch_t();
		bunch.Pos=pos;
		bunch.Type=pb_enum.BunchA;
		bunch.Pawns.Add(id);
		if(pos==Main.Instance.MainPlayer.playData.Seat&&!showHints(bunch)){
			yield return new WaitForSeconds(Configs.OpsInterval);
			//collect
			var omsg=new MsgCNMeld();
			omsg.Mid=pb_msg.MsgCnMeld;
			omsg.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsg.Mid,omsg);
			Debug.Log(pos+" collect "+id);
		}
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		_hints=null;

		var bunch=msg.Bunch;
		var from=msg.From;
		var to=bunch.Pos;
		var scalar=1;//(to==_pos?DiscardScalar:AbandonScalar);
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();
		if(bunch.Type==pb_enum.BunchAaaa){
			//could be startup AAAA,where A==null
			if(A==null){
				var hands=HandAreas[to].GetComponentsInChildren<Card>();
				foreach(var card in hands){
					if(card.Value==bunch.Pawns[0]){
						A=card;
						break;
					}
				}
			}
		}
		if(A==null)
			yield break;

		List<bunch_t> meldBunch=new List<bunch_t>();
		switch(bunch.Type){
		case pb_enum.BunchA:
			//collect
			if(_pos!=to){
				//remove extra card of other player
				var hand=HandAreas[to].GetComponentInChildren<Card>();
				if(hand!=null)
					Destroy(hand.gameObject);
			}
			A.DiscardTo(HandAreas[to],scalar);
			A.Static=false;
			A.state=Card.State.ST_NORMAL;
			if(to==_pos)StartCoroutine(sortHands());
			//draw with AAAA,unpack
			meldBunch.AddRange(bunch.Child);
			break;
		case pb_enum.BunchAbc:
		case pb_enum.BunchAaa:
		case pb_enum.BunchAaaa:
			//meld
			meldBunch.Add(bunch);
			break;
		default:
			//abandon
			if(to==-1)to=msg.From;
			A.DiscardTo(AbandonAreas[to],AbandonScalar);
			break;
		}
		//do meld
		foreach(var meld in meldBunch){
			//var melds=new List<Card>();
			if(_pos==to){
				//move cards to meld area
				var hands=HandAreas[to].GetComponentsInChildren<Card>();
				foreach(var id in meld.Pawns)
				foreach(var card in hands){
					if(card.Value==id && A.Value!=id)
						Destroy(card.gameObject);
						//melds.Add(card);
				}
			}else{
				//remove extra cards of other player
				var hands=HandAreas[to].GetComponentsInChildren<Card>();
				var rm=Mathf.Min(hands.Length,meld.Pawns.Count-1);
				for(int i=0;i<rm;++i){
					var hand=hands[i];
					Destroy(hand.gameObject);
				}
			}
			Destroy(A.gameObject);
			MahjongBunch mjBunch=null;
			Utils.Load<MahjongBunch>(MeldAreas[to],delegate(Component obj){
				mjBunch=obj as MahjongBunch;
				mjBunch.transform.SetSiblingIndex(0);
			});
			while(!mjBunch)yield return null;
			mjBunch.Value=meld;
		}

		//remove from hands
		if(player.playData.Seat==bunch.Pos){
			Rule.Meld(player,bunch);
		}

		yield return StartCoroutine(base.OnMsgMeld(player,msg));
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));

		for(int i=0;i<MeldAreas.Length;++i)foreach(Transform ch in MeldAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<AbandonAreas.Length;++i)foreach(Transform ch in AbandonAreas[i].transform)Destroy(ch.gameObject);
		Utils.Load<MahjongSettle>(Main.Instance.RootPanel,delegate(Component obj) {
			var popup=obj as SettlePopup;
			popup.Value=msg;
		});
	}

	override protected bool showHints(bunch_t bunch,bool startup=false){
		var player=Main.Instance.MainPlayer;

		_hints=Rule.Hint(player,bunch);

		//show/hide buttons
		var bbb=false;
		var bbbb=false;
		var win=false;
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.BunchA:
				//draw with AAAA
				if(b.Child.Count>0)
					bbbb=true;
				break;
			case pb_enum.BunchAaa:
				bbb=true;
				break;
			case pb_enum.BunchAaaa:
				bbbb=true;
				break;
			case pb_enum.BunchWin:
				win=true;
				break;
			default:
				break;
			}
		}
		if(win)BtnWin.SetActive(true);
		if(bbbb)BtnA4.SetActive(true);
		if(bbb&&!startup)BtnA3.SetActive(true);

		if(win||bbbb||bbb&&!startup)BtnPass.SetActive(true);

		return _hints.Count>0;
	}

	override protected IEnumerator sortHands(){
		var hands=HandAreas[_pos].GetComponentsInChildren<Card>();
		var ids=new List<int>();
		foreach(var card in hands)ids.Add(card.Value);
		ids.Sort(Rule.comparision);
		foreach(var card in hands){
			for(int i=0;i<ids.Count;++i)
				if(card.Value==ids[i]){
				card.transform.SetSiblingIndex(i);
				break;
			}
		}
		yield break;
	}
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		Rule=new MahJongRule();
		base.Awake();

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

		btnOps=new GameObject[]{BtnA3,BtnA4,BtnWin,BtnPass};
	}

	public void OnTong(){
		setDefaultColor(1001);
	}
	public void OnTiao(){
		setDefaultColor(2001);
	}
	public void OnWan(){
		setDefaultColor(3001);
	}

	void setDefaultColor(int key){
		var btns=new GameObject[]{BtnTong,BtnTiao,BtnWan};
		foreach(var btn in btns)btn.SetActive(false);
		//set default color
		var msg=new MsgCNEngage();
		msg.Mid=pb_msg.MsgCnEngage;
		msg.Key=key;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(msg.Mid,msg);
		Debug.Log("Set default card "+key.ToString());
	}

	public void OnAAA(){
		foreach(var btn in btnOps)btn.SetActive(false);
		if(_hints!=null&&_hints.Count>0){
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.BunchAaa){
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
			bunch_t bunch=null;
			bunch_t bunchA=null;
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.BunchAaaa)
					bunch=hint;
				else if(hint.Type==pb_enum.BunchA&&hint.Child.Count>0){
					//only support one currently
					//TODO: selection for multiples
					var ch0=hint.Child[0];
					hint.Child.Clear();
					hint.Child.Add(ch0);

					bunchA=hint;
				}
			}
			if(bunch==null)bunch=bunchA;
			if(bunch!=null){
				MsgCNMeld msg=new MsgCNMeld();
				msg.Mid=pb_msg.MsgCnMeld;
				msg.Bunch=new bunch_t();
				msg.Bunch.Pos=_pos;
				msg.Bunch.Type=bunch.Type;
				msg.Bunch.MergeFrom(bunch);
				Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
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
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string CardPrefab{get{return "Mahjong";}}
	override public string Id2File(int color,int value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"tong","tiao","wan"};
			value=Rule.inverseTransformValue(value);
			if(color<Colors.Length)
				return CardPrefab+"/"+string.Format("{0}{1:0}",Colors[color],value);
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
		Utils.Load<MahJongPanel>(Main.Instance.RootPanel,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
