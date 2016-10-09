using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public abstract class GamePanel : MonoBehaviour,GameController {
	public Card[]		BottomCards;
	public Transform	Pile;
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
		StartCoroutine(Discard());
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
		changeToken(msg.Pos);
		_banker=msg.Banker;
		_historical.Clear();
		_selection.Clear();
		/* position transform
		*	  (O)
		(R)          (L)
		*	(M=_pos)
		*/
		var m=maxPlayer-1;
		var M=_pos;
		var R=(M+1)%maxPlayer;
		var O=(M+2)%maxPlayer;
		var L=(M+m)%maxPlayer;
		Transform[] tempD=new Transform[DiscardAreas.Length];	//MROL
		Transform[] tempH=new Transform[HandAreas.Length];
		PlayerIcon[] tempP=new PlayerIcon[Players.Length];
		Text[] tempN=new Text[nHandCards.Length];
		DiscardAreas.CopyTo(tempD,0);
		HandAreas.CopyTo(tempH,0);
		Players.CopyTo(tempP,0);
		nHandCards.CopyTo(tempN,0);
		if(DiscardAreas.Length>0)DiscardAreas[0]=tempD[M];
		if(DiscardAreas.Length>1)DiscardAreas[1]=tempD[R];
		if(DiscardAreas.Length>2)DiscardAreas[2]=tempD[O];
		if(DiscardAreas.Length>m)DiscardAreas[m]=tempD[L];
		if(HandAreas.Length>0)HandAreas[0]=tempH[M];
		if(HandAreas.Length>1)HandAreas[1]=tempH[R];
		if(HandAreas.Length>2)HandAreas[2]=tempH[O];
		if(HandAreas.Length>m)HandAreas[m]=tempH[L];
		if(Players.Length>0)Players[0]=tempP[M];
		if(Players.Length>1)Players[1]=tempP[R];
		if(Players.Length>2)Players[2]=tempP[O];
		if(Players.Length>m)Players[m]=tempP[L];
		if(nHandCards.Length>0)nHandCards[0]=tempN[M];
		if(nHandCards.Length>1)nHandCards[1]=tempN[R];
		if(nHandCards.Length>2)nHandCards[2]=tempN[O];
		if(nHandCards.Length>m)nHandCards[m]=tempN[L];

		//sort
		var hands=new List<uint>(msg.Hands);
		hands.Sort(Rule.comparision);
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

		start();
		yield break;
	}

	public IEnumerator OnMsgDiscard(MsgNCDiscard msg){
		//discard any body's card
		if(msg.Result!=pb_enum.Succeess){
			yield break;
		}

		var pos=msg.Bunch.Pos;
		changeToken(pos);
		_hints.Clear();
		_hints.AddRange(msg.Hints);
		string str=pos+" discard ";
		var cards=new uint[msg.Bunch.Pawns.Count];
		msg.Bunch.Pawns.CopyTo(cards,0);
		if(pos==_pos){
			//adjust by feedback
			for(int i=0;i<cards.Length;++i){
				var id=(int)cards[i];
				str+=id+",";
			}
		}else{
			//remove discards
			foreach(Transform ch in DiscardAreas[pos].transform)Destroy(ch.gameObject);
			for(int i=0;i<cards.Length;++i){
				var id=cards[i];
				var fin=false;
				var from=(pos<nHandCards.Length&&nHandCards[pos]!=null?nHandCards[pos].transform.parent:HandAreas[pos]);
				Card.Create(CardPrefab,id,from,delegate(Card card) {
					card.state=Card.State.ST_DISCARD;
					card.DiscardTo(DiscardAreas[pos],DiscardScalar);
					fin=true;
				});
				yield return null;
				str+=(int)id+",";
				while(!fin)yield return null;
			}
			//yield return new WaitForSeconds(1);
		}
		//record
		if(pos<nHandCards.Length&&nHandCards[pos]!=null){
			var nCards=int.Parse(nHandCards[pos].text)-1;
			nHandCards[pos].text=nCards.ToString();
		}
		_historical.Add(msg.Bunch);
		discard(pos);
		Debug.Log(str);
	}
	
	public void OnMsgDraw(MsgNCDraw msg){
		changeToken(msg.Pos);
		_hints.Clear();
		_hints.AddRange(msg.Hints);
		if(_hints.Count>0)
			showHints();
		draw(msg.Card,msg.Pos);
		Debug.Log(msg.Pos+" draw "+(int)msg.Card);
	}
	public void OnMsgMeld(MsgNCMeld msg){
		changeToken(msg.Bunch.Pos);
		meld(msg.Bunch);
		Debug.Log(msg.Bunch.Pos+" meld "+(int)msg.Bunch.Type);
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

	public IEnumerator Discard(Card card=null){
		//discard my card
		if(checkDiscard(card)){
			//remove discards
			foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
			//discard
			MsgCNDiscard msg=new MsgCNDiscard();
			msg.Mid=pb_msg.MsgCnDiscard;
			msg.Bunch=new bunch_t();
			msg.Bunch.Pos=_pos;
			msg.Bunch.Type=pb_enum.BunchA;
			if(card!=null){
				deselectAll();
				card.state=Card.State.ST_DISCARD;
				card.DiscardTo(DiscardAreas[_pos],DiscardScalar);
				msg.Bunch.Pawns.Add(card.Value);
			}else if(_selection.Count>0){
				_selection.Sort(compare_card);
				foreach(var c in _selection){
					c.state=Card.State.ST_DISCARD;
					c.DiscardTo(DiscardAreas[_pos],DiscardScalar);
					msg.Bunch.Pawns.Add(c.Value);
				}
				_selection.Clear();
			}
			yield return new WaitForSeconds(1);
			Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
		}
	}

	virtual protected void start(){}
	virtual protected void discard(uint pos){}
	virtual protected void draw(uint card,uint pos){}
	virtual protected void meld(bunch_t bunch){}
	virtual protected void sortHands(){}

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

	protected void changeToken(uint pos){
		var old=_token;
		_token=pos%maxPlayer;
		if(old!=_token){
			if(Players[old].gameTimer!=null)
				Players[old].gameTimer.On(false);
			if(Players[_token].gameTimer!=null)
				Players[_token].gameTimer.On();
		}
	}
}
