using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public abstract class GamePanel : MonoBehaviour,GameController {
	public Card[]		BottomCards;
	public Transform	Pile;
	public Transform[]	HandAreas;
	public Transform[]	DiscardAreas;	//MROL(Me,Right,Opposite,Left)
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;
	public GameObject	BtnPass,Buttons;

	protected int		maxPlayer=0;
	protected int		round=0;
	protected GameRule	rule=null;

	protected int			_pos,_token,_banker;
	protected List<Card>	_selection=new List<Card>();

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

	virtual public void OnPass(){
		deselectAll();
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.OpPass;
		Main.Instance.MainPlayer.Send<MsgCNDiscard>(msg.Mid,msg);
	}
	
	public void OnExit(){
		Utils.Load<LobbyPanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}
	// ----------------------------------------------
	// messages
	// ----------------------------------------------
	public void onMessage(Player player,IMessage msg){
		if(msg is MsgNCEngage){
			var msgEngage=msg as MsgNCEngage;
			if(player.pos==msgEngage.Pos)
				onMsgEngage(msgEngage);

		}else if(msg is MsgNCStart){
			var msgStart=msg as MsgNCStart;
			StartCoroutine(OnMsgStart(msgStart));

		}else if(msg is MsgNCDiscard){
			var msgDiscard=msg as MsgNCDiscard;
			StartCoroutine(OnMsgDiscard(msgDiscard));

		}else if(msg is MsgNCMeld){
			var msgMeld=msg as MsgNCMeld;
			changeToken(msgMeld.Bunch.Pos);
			onMsgMeld(msgMeld.Bunch);

		}else if(msg is MsgNCDraw){
			var msgDraw=msg as MsgNCDraw;
			Debug.Log(msgDraw.Pos+" draw "+(int)msgDraw.Card);
			changeToken(msgDraw.Pos);
			onMsgDraw(msgDraw.Card,msgDraw.Pos);

		}else if(msg is MsgNCSettle){
			var msgSettle=msg as MsgNCSettle;
			OnMsgSettle(msgSettle);

		}else if(msg is MsgNCFinish){
			var msgFinish=msg as MsgNCFinish;
			OnMsgFinish(msgFinish);

		}else if(msg is MsgNCDismissSync){

		}else if(msg is MsgNCDismissAck){

		}
	}

	public IEnumerator OnMsgStart(MsgNCStart msg){
		while(!CardCache.Ready||maxPlayer<=0)yield return null;
		++Round;
		_pos=msg.Pos;
		changeToken(msg.Pos);
		_banker=msg.Banker;
		Rule.Historical.Clear();
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
		var hands=new List<int>(msg.Hands);
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

		onMsgStart();
		yield break;
	}

	public IEnumerator OnMsgDiscard(MsgNCDiscard msg){
		//discard any body's card
		if(msg.Result!=pb_enum.Succeess){
			yield break;
		}

		var pos=msg.Bunch.Pos;
		changeToken(pos);
		string str=pos+" discard ";
		var cards=new int[msg.Bunch.Pawns.Count];
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
		Debug.Log(str);
		//record
		if(pos<nHandCards.Length&&nHandCards[pos]!=null){
			var nCards=int.Parse(nHandCards[pos].text)-1;
			nHandCards[pos].text=nCards.ToString();
		}
		onMsgDiscard(msg);
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
	public int Round{get{return round;}set{round=value;}}
	public GameRule Rule{get{return rule;}set{rule=value;}}
	abstract public string Id2File(int color,int value);
	abstract public float DiscardScalar{get;}
	abstract public string CardPrefab{get;}
	
	virtual public void TapCard(Card card,bool select=true){
		if(select)
			_selection.Add(card);
		else
			_selection.Remove(card);
		card.Tap();
	}

	public IEnumerator Discard(Card card=null){
		//discard my card
		if(null==card&&_selection.Count<=0){
			Debug.Log("Discard invalid card");
			yield break;
		}
		
		bunch_t curr=new bunch_t();
		curr.Pos=_pos;
		if(card!=null)
			curr.Pawns.Add(card.Value);
		else foreach(var c in _selection)
			curr.Pawns.Add(c.Value);

		if(Rule.verifyDiscard(Main.Instance.MainPlayer,curr)){
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
			yield return new WaitForSeconds(Configs.OpsInterval);
			Main.Instance.MainPlayer.Send<MsgCNDiscard>(msg.Mid,msg);
		}else{
			Debug.LogError("Discard error verify failed");
		}
	}

	virtual protected void onMsgEngage(MsgNCEngage msg){}
	virtual protected void onMsgStart(){}
	virtual protected void onMsgDiscard(MsgNCDiscard msg){}
	virtual protected void onMsgDraw(int card,int pos){}
	virtual protected void onMsgMeld(bunch_t bunch){}
	virtual protected void sortHands(){}

	protected void deselectAll(){
		var copy=new List<Card>(_selection);
		foreach(var c in copy)c.Tap();	//avoid crash
		_selection.Clear();
	}


	//System.Comparison<int>
	int compare_card(Card x,Card y){
		return rule.comparision(x.Value,y.Value);
	}

	protected void changeToken(int pos){
		if(pos==-1)return;
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
