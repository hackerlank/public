﻿using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public abstract class GamePanel : MonoBehaviour,GameController {
	public Card[]		BottomCards;
	public Transform[]	HandAreas;
	public Transform[]	DiscardAreas;	//MROL(Me,Right,Opposite,Left)
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;
	public GameObject	BtnHint,BtnDiscard,BtnPass,Buttons;

	protected uint		maxPlayer=0;
	protected uint		round=0;
	protected GameRule	rule=null;

	protected uint			_pos,_token,_banker;
	protected List<Card>	_selection=new List<Card>();
	protected List<bunch_t>	_hints=new List<bunch_t>();
	protected List<bunch_t>	_historical=new List<bunch_t>();

	public MsgNCFinish	Summary=null;
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	virtual public void Awake(){
		Main.Instance.gameController=this;
	}

	IEnumerator Start(){
		maxPlayer=rule.MaxPlayer;
		while(!CardCache.Ready)yield return null;
	}

	void OnDestroy(){
		Main.Instance.gameController=null;
	}

	public void OnDiscard(){
		Discard();
	}
	
	//int _nhints=0;
	public void OnHint(){
		deselectAll();
		/*
		if(_hints==null)genHints();

		if(_hints!=null&&_hints.Count>0){
			var hints=_hints[_nhints];
			foreach(Transform ch in HandArea.transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)foreach(var id in hints)
					if(card.Value.Id==id)
						card.Tap();
			}
			_nhints=(_nhints+1)%_hints.Count;
		}
		*/
	}
	
	virtual public void OnPass(){
		deselectAll();
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.OpPass;
		Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
	}
	
	public void OnExit(){
		Utils.Load<LobbyPanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}
	// ----------------------------------------------
	// messages
	// ----------------------------------------------
	public IEnumerator OnMsgStart(MsgNCStart msg){
		while(!CardCache.Ready||maxPlayer<=0)yield return null;
		++Round;
		_pos=msg.Pos;
		_token=(msg.Banker+maxPlayer-1)%maxPlayer;	//set to the previous position
		_banker=msg.Banker;
		_historical.Clear();
		_selection.Clear();
		/* position transform
		*	  (O)
		(R)          (L)
		*	(M=_pos)
		*/
		var M=_pos;
		var R=(M+1)%maxPlayer;
		var L=(M+2)%maxPlayer;
		var O=(M+3)%maxPlayer;
		Transform[] tempD=new Transform[DiscardAreas.Length];	//MRL
		PlayerIcon[] tempP=new PlayerIcon[Players.Length];
		Text[] tempH=new Text[nHandCards.Length];
		DiscardAreas.CopyTo(tempD,0);
		Players.CopyTo(tempP,0);
		nHandCards.CopyTo(tempH,0);
		if(DiscardAreas.Length>0)DiscardAreas[0]=tempD[M];
		if(DiscardAreas.Length>1)DiscardAreas[1]=tempD[R];
		if(DiscardAreas.Length>2)DiscardAreas[2]=tempD[L];
		if(DiscardAreas.Length>3)DiscardAreas[3]=tempD[O];
		if(Players.Length>0)Players[0]=tempP[M];
		if(Players.Length>1)Players[1]=tempP[R];
		if(Players.Length>2)Players[2]=tempP[L];
		if(Players.Length>3)Players[3]=tempP[O];
		if(nHandCards.Length>0)nHandCards[0]=tempH[M];
		if(nHandCards.Length>1)nHandCards[1]=tempH[R];
		if(nHandCards.Length>2)nHandCards[2]=tempH[L];
		if(nHandCards.Length>3)nHandCards[3]=tempH[O];
		
		//sort
		var hands=new List<uint>(msg.Hands);
		hands.Sort(rule.comparision);
		//deal
		
		string str="deal: banker="+msg.Banker+",pos="+msg.Pos+",hands:\n";
		for(int i=0;i<hands.Count;++i){
			var id=hands[i];
			var fin=false;
			Card.Create(CardPrefab,id,HandAreas[0],delegate(Card card) {
				card.Static=false;
				fin=true;
			});
			yield return null;
			str+=id.ToString()+",";
			if((i+1)%6==0)str+="\n";
			while(!fin)yield return null;
		}
		Ante.text=string.Format("Ante: {0}",msg.Ante);
		Multiples.text=string.Format("Multiple: {0}",msg.Multiple);
		for(int i=0;i<msg.Count.Count;++i)
			if(i<nHandCards.Length&&nHandCards[i]!=null)nHandCards[i].text=msg.Count[i].ToString();
		for(int i=0;i<msg.Bottom.Count;++i){
			if(i<BottomCards.Length&&BottomCards[i]!=null)
				BottomCards[i].Value=msg.Bottom[i];
		}
		if(Players[_banker].gameTimer!=null)
			Players[_banker].gameTimer.On();
		Debug.Log(str);
		yield break;
	}

	public IEnumerator OnMsgDiscard(MsgNCDiscard msg){
		//discard any body's card
		if(msg.Result!=pb_enum.Succeess){
			yield break;
		}
		
		_token=msg.Bunch.Pos;
		_hints.Clear();
		_hints.AddRange(msg.Hints);
		string str="discard at "+_token;
		var cards=new uint[msg.Bunch.Pawns.Count];
		msg.Bunch.Pawns.CopyTo(cards,0);
		if(_token==_pos){
			//adjust by feedback
		}else{
			//remove discards
			if(Rule.removeDiscard)foreach(Transform ch in DiscardAreas[_token].transform)Destroy(ch.gameObject);
			for(int i=0;i<cards.Length;++i){
				var id=cards[i];
				var fin=false;
				var from=(_token<nHandCards.Length&&nHandCards[_token]!=null?nHandCards[_token].transform.parent:HandAreas[_token]);
				Card.Create(CardPrefab,id,from,delegate(Card card) {
					card.state=Card.State.ST_DISCARD;
					card.DiscardTo(DiscardAreas[_token]);
					fin=true;
				});
				yield return null;
				str+=id.ToString()+",";
				while(!fin)yield return null;
			}
		}
		//record
		if(_token<nHandCards.Length&&nHandCards[_token]!=null){
			var nCards=int.Parse(nHandCards[_token].text)-1;
			nHandCards[_token].text=nCards.ToString();
		}
		_historical.Add(msg.Bunch);
		next(_token);
		//Debug.Log(str);
	}
	
	public void OnMsgDraw(MsgNCDraw msg){
		_hints.Clear();
		_hints.AddRange(msg.Hints);
		if(_hints.Count>0)
			showHints();
	}
	public void OnMsgMeld(MsgNCMeld msg){
		if(msg.Result==pb_enum.Succeess)meld(msg.Bunch);
	}

	public void OnMsgSettle(MsgNCSettle msg){
		for(int i=0;i<DiscardAreas.Length;++i)foreach(Transform ch in DiscardAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<HandAreas.Length;++i)foreach(Transform ch in HandAreas[i].transform)Destroy(ch.gameObject);
		Utils.Load<SettlePopup>(Main.Instance.transform);
	}

	public void OnMsgFinish(MsgNCFinish msg){
		Summary=msg;
	}
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	public uint Round{get{return round;}set{round=value;}}
	public GameRule Rule{get{return rule;}set{rule=value;}}
	abstract public string Id2File(uint color,uint value);
	abstract public float DiscardScalar{get;}
	abstract public string CardPrefab{get;}
	
	virtual protected bool checkDiscard(Card card=null){return true;}
	
	virtual public void TapCard(Card card,bool select=true){
		if(select)
			_selection.Add(card);
		else
			_selection.Remove(card);
		card.Tap();
	}

	public void Discard(Card card=null){
		//discard my card
		if(checkDiscard(card)){
			//remove discards
			if(Rule.removeDiscard)foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
			//discard
			MsgCNDiscard msg=new MsgCNDiscard();
			msg.Mid=pb_msg.MsgCnDiscard;
			msg.Bunch=new bunch_t();
			msg.Bunch.Pos=_pos;
			msg.Bunch.Type=pb_enum.BunchA;
			if(card!=null){
				deselectAll();
				card.state=Card.State.ST_DISCARD;
				card.DiscardTo(DiscardAreas[_pos]);
				msg.Bunch.Pawns.Add(card.Value);
			}else if(_selection.Count>0){
				_selection.Sort(compare_card);
				foreach(var c in _selection){
					c.state=Card.State.ST_DISCARD;
					c.DiscardTo(DiscardAreas[_pos]);
					msg.Bunch.Pawns.Add(c.Value);
				}
				_selection.Clear();
			}
			Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
		}
	}

	virtual protected void meld(bunch_t bunch){}

	virtual protected void genHints(){}
	virtual protected void showHints(){}

	protected void deselectAll(){
		var copy=new List<Card>(_selection);
		foreach(var c in copy)c.Tap();	//avoid crash
		_selection.Clear();
	}


	//System.Comparison<uint>
	int compare_card(Card x,Card y){
		return rule.comparision(x.Value,y.Value);
	}

	void next(uint pos){
		var R=(pos+1)%maxPlayer;
		if(Players[pos].gameTimer!=null)
			Players[pos].gameTimer.On(false);
		if(Players[R].gameTimer!=null)
			Players[R].gameTimer.On();
	}
}
