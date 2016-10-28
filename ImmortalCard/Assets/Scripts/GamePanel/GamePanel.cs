using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public abstract class GamePanel : MonoBehaviour,GameController,IPointerDownHandler,IPointerUpHandler{
	public Card[]		BottomCards;
	public Transform	Pile;
	public Transform[]	HandAreas;
	public Transform[]	DiscardAreas;	//MROL(Me,Right,Opposite,Left)
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;
	public GameObject	BtnPass,Buttons;

	protected GameObject[]		btnOps;	//all ops buttons

	protected int		maxPlayer=0;
	protected int		round=0;
	protected GameRule	rule=null;

	protected int			_pos;
	protected List<Card>	_selection=new List<Card>();

	public MsgNCFinish	Summary=null;
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	virtual public void Awake(){
		Main.Instance.gameController=this;
		Main.Instance.MainPlayer.controllers.Add(this);
	}

	IEnumerator Start(){
		maxPlayer=rule.MaxPlayer;
		while(!CardCache.Ready)yield return null;
	}

	void OnDestroy(){
		Main.Instance.MainPlayer.Disconnect();
		foreach(var robot in Main.Instance.robots)robot.Disconnect();
		Main.Instance.robots.Clear();
		Main.Instance.MainPlayer.controllers.Clear();
		Main.Instance.gameController=null;
	}

	virtual public void OnPass(){
		foreach(var btn in btnOps)btn.SetActive(false);
		deselectAll();
		StartCoroutine(passMeld(Main.Instance.MainPlayer,0,false));
	}
	
	public void OnExit(){
		Utils.Load<LobbyPanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}

	bool pointerDown=false;
	public void OnPointerDown (PointerEventData eventData){
		pointerDown=true;
	}

	public void OnPointerUp (PointerEventData eventData){
		pointerDown=false;
	}

	public void OnCardEnter(Card card){
		if(pointerDown)
			TapCard(card,card.state!=Card.State.ST_SELECT);
	}
	// ----------------------------------------------
	// messages
	// ----------------------------------------------
	public IEnumerator onMessage(Player player,IMessage msg){
		if(msg is MsgNCEngage){
			var msgEngage=msg as MsgNCEngage;
			OnMsgEngage(msgEngage);

		}else if(msg is MsgNCStart){
			var msgStart=msg as MsgNCStart;
			yield return StartCoroutine(OnMsgStart(msgStart));

		}else if(msg is MsgNCDiscard){
			var msgDiscard=msg as MsgNCDiscard;
			StartCoroutine(OnMsgDiscard(msgDiscard));

		}else if(msg is MsgNCMeld){
			var msgMeld=msg as MsgNCMeld;
			StartCoroutine(OnMsgMeld(msgMeld.Bunch));
			changeToken(msgMeld.Bunch.Pos);

		}else if(msg is MsgNCDraw){
			var msgDraw=msg as MsgNCDraw;
			Debug.Log(msgDraw.Pos+" draw "+(int)msgDraw.Card);
			changeToken(msgDraw.Pos);
			OnMsgDraw(msgDraw.Card,msgDraw.Pos);

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

	protected void transformComponent(Component[] com){
		/* position transform
		*	  (O)
		(R)          (L)
		*	(M=_pos)
		*/
		var tempD=new Component[com.Length];	//MROL
		com.CopyTo(tempD,0);

		//turn components clockwise by _pos
		var m=maxPlayer-1;
		var maxArea=com.Length;
		var M=(maxArea+0-_pos)%maxArea;
		var R=(maxArea+1-_pos)%maxArea;
		var O=(maxArea+2-_pos)%maxArea;
		var L=(maxArea-1-_pos)%maxArea;
		
		if(com.Length>0)com[0]=tempD[M];
		if(com.Length>1)com[1]=tempD[R];
		if(com.Length>2)com[2]=tempD[O];
		if(com.Length>m)com[m]=tempD[L];
	}

	protected void checkNaturalWin(){
		//check natural win
		var players=new List<Player>(Main.Instance.robots);
		players.Add(Main.Instance.MainPlayer);
		foreach(var player in players){
			var hands=player.playData.Hands;
			var last=hands[hands.Count-1];
			hands.RemoveAt(hands.Count-1);
			
			bunch_t bunch=new bunch_t();
			bunch.Pos=player.pos;
			bunch.Type=pb_enum.BunchWin;
			bunch.Pawns.Add(last);
			
			var win=false;
			if(player==Main.Instance.MainPlayer){
				win=showHints(last,true,true);
			}else{
				var hints=Rule.Hint(player,bunch);
				foreach(var h in hints){
					if(h.Type>=pb_enum.BunchWin){
						win=true;
						break;
					}
				}
			}
			hands.Add(last);
			
			if(player!=Main.Instance.MainPlayer||!win){
				if(!win)bunch.Type=pb_enum.OpPass;
				else Debug.Log(player.pos+" natural win");
				
				var omsgMeld=new MsgCNMeld();
				omsgMeld.Mid=pb_msg.MsgCnMeld;
				omsgMeld.Bunch=bunch;
				
				player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
			}
		}
	}

	public IEnumerator OnMsgStart(MsgNCStart msg){
		while(!CardCache.Ready||maxPlayer<=0)yield return null;
		++Round;
		_pos=msg.Pos;
		changeToken(msg.Pos);
		Rule.Banker=msg.Banker;
		Rule.Historical.Clear();
		_selection.Clear();

		transformComponent(DiscardAreas);
		transformComponent(HandAreas);
		transformComponent(Players);
		transformComponent(nHandCards);

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
		if(Players[Rule.Banker].gameTimer!=null)
			Players[Rule.Banker].gameTimer.On();
		Debug.Log(str);

		onMsgStart();
		yield break;
	}

	IEnumerator OnMsgDiscard(MsgNCDiscard msg){
		//discard any body's card
		if(msg.Result==pb_enum.Succeess){
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
						card.DiscardTo(DiscardAreas[pos],DiscardScalar);
						card.state=Card.State.ST_DISCARD;
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
	virtual public bool CardDrag{get{return true;}}
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
				card.DiscardTo(DiscardAreas[_pos],DiscardScalar);
				card.state=Card.State.ST_DISCARD;
				msg.Bunch.Pawns.Add(card.Value);
			}else if(_selection.Count>0){
				_selection.Sort(compare_card);
				foreach(var c in _selection){
					c.DiscardTo(DiscardAreas[_pos],DiscardScalar);
					c.state=Card.State.ST_DISCARD;
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

	virtual protected void OnMsgEngage(MsgNCEngage msg){}
	virtual protected void onMsgStart(){}
	virtual protected void onMsgDiscard(MsgNCDiscard msg){}
	virtual protected void OnMsgDraw(int card,int pos){}
	virtual protected IEnumerator OnMsgMeld(bunch_t bunch){yield break;}
	virtual protected IEnumerator sortHands(){yield break;}
	virtual protected bool showHints(int card,bool bDraw,bool startup=false){return true;}

	protected IEnumerator passDiscard(Player player,bool wait=true){
		if(wait)yield return new WaitForSeconds(Configs.OpsInterval);
		
		//pass discard or draw
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;

		var bunch=new bunch_t();
		bunch.Pos=player.pos;
		bunch.Type=pb_enum.OpPass;
		msg.Bunch=bunch;

		Main.Instance.MainPlayer.Send<MsgCNDiscard>(msg.Mid,msg);
	}

	protected IEnumerator passMeld(Player player,int card=0,bool wait=true){
		if(wait)yield return new WaitForSeconds(Configs.OpsInterval);
		
		//pass discard or draw
		var msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		
		bunch_t bunch=new bunch_t();
		bunch.Pos=player.pos;
		bunch.Pawns.Add(card);
		bunch.Type=pb_enum.OpPass;
		msg.Bunch=bunch;
		
		player.Send<MsgCNMeld>(msg.Mid,msg);
	}

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
		var old=Rule.Token;
		Rule.Token=pos%maxPlayer;
		if(old!=Rule.Token){
			if(Players[old].gameTimer!=null)
				Players[old].gameTimer.On(false);
			if(Players[Rule.Token].gameTimer!=null)
				Players[Rule.Token].gameTimer.On();
		}
	}
}