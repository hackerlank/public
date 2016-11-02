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

	float AbandonScalar{get{return .7f;}}
	override public float DiscardScalar{get{return 1f;}}
	
	public override void TapCard(Card card,bool select=true){
		var selected=false;
		foreach(var old in _selection)if(old==card)selected=true;
		deselectAll();
		if(!selected)base.TapCard(card,select);
	}

	override public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		yield return StartCoroutine(base.OnMsgStart(player,msg));
		//transform position
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);
	}

	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		GameObject[] btns=new GameObject[]{BtnTong,BtnTiao,BtnWan};
		foreach(var btn in btns)btn.SetActive(false);
		
		for(int i=0;i<msg.Keys.Count;++i)
			if(Main.Instance.MainPlayer.pos==i)
				Main.Instance.MainPlayer.playData.SelectedCard=msg.Keys[i];
		
		checkNaturalWin();
		yield break;
	}

	List<bunch_t> _hints=null;
	override public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		yield return StartCoroutine(base.OnMsgDiscard(player,msg));
		//show hints for others
		if(msg.Bunch.Pawns.Count>0&&Main.Instance.MainPlayer.pos!=msg.Bunch.Pos){
			var card=msg.Bunch.Pawns[0];
			if(!showHints(card,false)){
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
		Card.Create(CardPrefab,id,Pile,delegate(Card card) {
			card.DiscardTo(DiscardAreas[pos],DiscardScalar);
			card.state=Card.State.ST_DISCARD;
		});

		//show hints only for MainPlayer
		if(pos==Main.Instance.MainPlayer.pos&&!showHints(id,true)){
			StartCoroutine(passMeld(Main.Instance.MainPlayer,id));
			Debug.Log(Main.Instance.MainPlayer.pos+" pass after self draw");
		}
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		_hints=null;
		
		var bunch=msg.Bunch;
		var from=Rule.Token;
		var to=bunch.Pos;
		var scalar=(to==_pos?DiscardScalar:AbandonScalar);
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();
		if(A==null)
			yield break;

		switch(bunch.Type){
		case pb_enum.BunchA:
			//collect
			A.DiscardTo(HandAreas[to],scalar);
			A.Static=false;
			A.state=Card.State.ST_NORMAL;
			if(to==_pos)StartCoroutine(sortHands());
			break;
		case pb_enum.BunchAaa:
		case pb_enum.BunchAaaa:
			//meld
			A.DiscardTo(MeldAreas[to],scalar);
			var hands=HandAreas[to].GetComponentsInChildren<Card>();
			foreach(var id in bunch.Pawns)
			foreach(var card in hands){
				if(card.Value==id)
					card.DiscardTo(MeldAreas[to],scalar);
			}
			break;
		default:
			//abandon
			if(to==-1)to=Rule.Token;
			A.DiscardTo(AbandonAreas[to],AbandonScalar);
			break;
		}
		//remove from hands
		if(player.pos==bunch.Pos){
			Rule.Meld(player,bunch);
		}

		yield return StartCoroutine(base.OnMsgMeld(player,msg));
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));

		for(int i=0;i<MeldAreas.Length;++i)foreach(Transform ch in MeldAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<AbandonAreas.Length;++i)foreach(Transform ch in AbandonAreas[i].transform)Destroy(ch.gameObject);
		Utils.Load<MahjongSettle>(Main.Instance.transform,delegate(Component obj) {
			var popup=obj as SettlePopup;
			popup.Value=msg;
		});
	}

	override protected bool showHints(int card,bool bDraw,bool startup=false){
		var player=Main.Instance.MainPlayer;
		var bunch=new bunch_t();
		bunch.Pos=(bDraw?player.pos:(player.pos+1)%maxPlayer);
		bunch.Type=pb_enum.BunchA;
		bunch.Pawns.Add(card);

		_hints=Rule.Hint(player,bunch);

		//show/hide buttons
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.BunchAaa:
				if(!startup)BtnA3.SetActive(true);
				break;
			case pb_enum.BunchAaaa:
				if(!startup)BtnA4.SetActive(true);
				break;
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
			foreach(var hint in _hints){
				if(hint.Type==pb_enum.BunchAaaa){
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
		Utils.Load<MahJongPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
