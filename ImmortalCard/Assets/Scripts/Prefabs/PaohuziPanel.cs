using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	public GameObject	BtnABC,BtnA3,BtnA4,BtnWin;
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string CardPrefab{get{return "Zipai";}}
	override public string Id2File(int color,int value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"b","s"};
			value=Rule.inverseTransformValue(value);
			if(color<Colors.Length)
				return string.Format("{0}{1:00}",Colors[color],value);
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

	override protected void onMsgStart(){
		//transform position
		var m=maxPlayer-1;
		var M=_pos;
		var R=(M+1)%maxPlayer;
		var O=(M+2)%maxPlayer;
		var L=(M+m)%maxPlayer;
		Transform[] tempM=new Transform[MeldAreas.Length];
		Transform[] tempA=new Transform[AbandonAreas.Length];
		MeldAreas.CopyTo(tempM,0);
		AbandonAreas.CopyTo(tempA,0);
		if(MeldAreas.Length>0)MeldAreas[0]=tempM[M];
		if(MeldAreas.Length>1)MeldAreas[1]=tempM[R];
		if(MeldAreas.Length>2)MeldAreas[2]=tempM[O];
		if(MeldAreas.Length>m)MeldAreas[m]=tempM[L];
		if(AbandonAreas.Length>0)AbandonAreas[0]=tempA[M];
		if(AbandonAreas.Length>1)AbandonAreas[1]=tempA[R];
		if(AbandonAreas.Length>2)AbandonAreas[2]=tempA[O];
		if(AbandonAreas.Length>m)AbandonAreas[m]=tempA[L];
	}

	List<bunch_t> _hints=null;
	override protected void onMsgDiscard(MsgNCDiscard msg){
		//show hints for others
		if(msg.Bunch.Pawns.Count>0&&Main.Instance.MainPlayer.pos!=msg.Bunch.Pos){
			var card=msg.Bunch.Pawns[0];
			if(!showHints(card,false)){
				StartCoroutine(passCo(card));
				Debug.Log(Main.Instance.MainPlayer.pos+" pass after "+msg.Bunch.Pos+" discard");
			}
		}
	}

	override protected void onMsgDraw(int id,int pos){
		//remove discards
		foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
		//discard
		Card.Create(CardPrefab,id,Pile,delegate(Card card) {
			card.DiscardTo(DiscardAreas[pos],DiscardScalar);
			card.state=Card.State.ST_DISCARD;
		});

		//show hints only for MainPlayer
		if(pos==Main.Instance.MainPlayer.pos&&!showHints(id,true)){
			StartCoroutine(passCo(id));
			Debug.Log(Main.Instance.MainPlayer.pos+" pass after self draw");
		}
	}

	override protected void onMsgMeld(bunch_t bunch){
		_hints=null;
		
		var from=Rule.Token;
		var to=bunch.Pos;
		var scalar=(to==_pos?DiscardScalar:AbandonScalar);
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();
		if(A!=null)
		switch(bunch.Type){
		case pb_enum.BunchA:
			//collect
			A.DiscardTo(HandAreas[to],scalar);
			A.Static=false;
			A.state=Card.State.ST_NORMAL;
			if(to==_pos)sortHands();
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
			A.state=Card.State.ST_MELD;
			break;
		default:
			//abandon
			if(to==-1)to=Rule.Token;
			A.DiscardTo(AbandonAreas[to],AbandonScalar);
			A.state=Card.State.ST_ABANDON;
			break;
		}
	}
	
	bool showHints(int card,bool bDraw){
		var player=Main.Instance.MainPlayer;
		var hands=new int[player.gameData.Hands.Count];
		player.gameData.Hands.CopyTo(hands,0);

		var bunch=new bunch_t();
		bunch.Pos=(bDraw?player.pos:player.pos+1);
		bunch.Type=pb_enum.BunchA;
		bunch.Pawns.Add(card);

		_hints=Rule.Hint(player,hands,bunch);

		//show/hide buttons
		//GameObject[] btns=new GameObject[]{BtnA3,BtnA4,BtnWin};
		//foreach(var btn in btns)btn.SetActive(false);
		foreach(var b in _hints){
			switch(b.Type){
			case pb_enum.BunchAa:
				break;
			case pb_enum.BunchAaa:
				break;
			case pb_enum.BunchAaaa:
				break;
			default:
				break;
			}
		}

		return _hints.Count>0;
	}

	IEnumerator passCo(int card){
		yield return new WaitForSeconds(Configs.OpsInterval);

		//pass discard or draw
		var player=Main.Instance.MainPlayer;
		var omsgMeld=new MsgCNMeld();
		omsgMeld.Mid=pb_msg.MsgCnMeld;
		
		bunch_t bunch=new bunch_t();
		bunch.Pos=player.pos;
		bunch.Pawns.Add(card);
		bunch.Type=pb_enum.OpPass;
		omsgMeld.Bunch=bunch;
		
		player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
	}
	
	override protected void sortHands(){
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
	}
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new PaohuziRule();

		var files=new List<string>();
		files.Add("back");
		for(int k=1;k<=2;++k)for(int i=1;i<=10;++i)
			files.Add(Id2File(k,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Zipai"));
	}

	public void OnABC(){
	}
	
	public void OnAAA(){
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
	
	override public void OnPass(){
		MsgCNMeld msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.OpPass;
		Main.Instance.MainPlayer.Send<MsgCNMeld>(msg.Mid,msg);
	}
	
	public static void Create(System.Action<Component> handler=null){
		Utils.Load<PaohuziPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
