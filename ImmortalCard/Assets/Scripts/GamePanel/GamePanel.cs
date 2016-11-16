using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public abstract class GamePanel : MonoBehaviour,GameController,IPointerDownHandler,IPointerUpHandler{
	public TokenIcon	tokenIcon;
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
	// messages
	// ----------------------------------------------
	virtual public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		while(!CardCache.Ready||maxPlayer<=0)yield return null;
		++Round;
		_pos=msg.Pos;
		changeToken(msg.Pos);
		Rule.Banker=msg.Banker;
		Rule.Historical.Clear();
		Rule.Pile.Clear();
		_selection.Clear();

		transformComponent(DiscardAreas);
		transformComponent(HandAreas);
		transformComponent(Players);
		transformComponent(nHandCards);

		//sort
		yield return StartCoroutine(deal(msg));

		//the other player
		for(int i=0;i<maxPlayer;++i){
			if(i==_pos)continue;
			if(i<HandAreas.Length && HandAreas[i]!=null &&
			   HandAreas[i].gameObject.activeSelf){
				var n=msg.Count[i];
				for(int j=0;j<n;++j){
					var fin=false;
					Card.Create(CardPrefab,1000,HandAreas[i],delegate(Card card) {
						fin=true;
					});
					yield return null;
					while(!fin)yield return null;
				}
			}
		}

		//other informations
		Ante.text=string.Format("Ante: {0}",msg.Ante);
		Multiples.text=string.Format("Multiple: {0}",msg.Multiple);
		for(int i=0;i<msg.Count.Count;++i)
			if(i<nHandCards.Length&&nHandCards[i]!=null)nHandCards[i].text=msg.Count[i].ToString();
		for(int i=0;i<msg.Bottom.Count;++i){
			if(i<BottomCards.Length&&BottomCards[i]!=null)
				BottomCards[i].Value=msg.Bottom[i];
		}
		tokenIcon.Token=rule.Banker;
		tokenIcon.Pile=msg.Piles;
	}

	virtual public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		yield break;
	}

	virtual public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
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

				//show new discards
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
			}
			Debug.Log(str);
			//remember
			if(pos<nHandCards.Length&&nHandCards[pos]!=null){
				var nCards=int.Parse(nHandCards[pos].text)-1;
				nHandCards[pos].text=nCards.ToString();
			}
		}
	}

	virtual public IEnumerator OnMsgDraw(Player player,MsgNCDraw msg){
		changeToken(msg.Pos);
		tokenIcon.Pile=tokenIcon.Pile-1;
		yield break;
	}
	
	virtual public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		changeToken(msg.Bunch.Pos);
		yield break;
	}
	
	virtual public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		for(int i=0;i<DiscardAreas.Length;++i)foreach(Transform ch in DiscardAreas[i].transform)Destroy(ch.gameObject);
		for(int i=0;i<HandAreas.Length;++i)foreach(Transform ch in HandAreas[i].transform)Destroy(ch.gameObject);
		yield break;
	}

	virtual public IEnumerator OnMsgFinish(Player player,MsgNCFinish msg){
		Summary=msg;
		yield break;
	}

	virtual protected IEnumerator deal(MsgNCStart msg){
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
		Debug.Log(str);
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
		//check natural win for banker
		var players=new List<Player>(Main.Instance.robots);
		players.Add(Main.Instance.MainPlayer);
		foreach(var player in players){
			if(player.playData.Seat!=Rule.Banker)continue;

			bunch_t bunch=new bunch_t();
			bunch.Pos=player.playData.Seat;
			bunch.Type=pb_enum.BunchA;
			bunch.Pawns.Add(Configs.invalidCard);
			
			var meld=false;
			if(player==Main.Instance.MainPlayer){
				meld=showHints(bunch,true);
			}else{
				//robots
				var hints=Rule.Hint(player,bunch);
				bunch_t bun=null;
				foreach(var h in hints){
					if(h.Type>=pb_enum.BunchWin){
						bun=h;
						break;
					}else if(h.Type==pb_enum.BunchAaaa){
						bun=h;
					}
				}
				if(bun!=null){
					meld=true;

					var msg=new MsgCNMeld();
					msg.Mid=pb_msg.MsgCnMeld;
					msg.Bunch=bun;
					player.Send<MsgCNMeld>(msg.Mid,msg);
				}
			}
			
			if(!meld)
				StartCoroutine(passMeld(player,Configs.invalidCard,true));
		}
	}
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
		Debug.Log("----- close game panel");
		Main.Instance.MainPlayer.Disconnect();
		foreach(var robot in Main.Instance.robots)robot.Disconnect();
		Main.Instance.robots.Clear();
		Main.Instance.MainPlayer.controllers.Clear();
		Main.Instance.gameController=null;
	}
	
	virtual public void OnPass(){
		foreach(var btn in btnOps)btn.SetActive(false);
		
		int id=Configs.invalidCard;
		var card=DiscardAreas[Rule.Token].GetComponentInChildren<Card>();
		if(card!=null){
			id=card.Value;
			Main.Instance.MainPlayer.unpairedCards.Add(id);
		}else{
			Debug.LogError("no card found when pass");
		}
		
		deselectAll();
		StartCoroutine(passMeld(Main.Instance.MainPlayer,id,false));
	}
	
	public void OnExit(){
		Utils.Load<LobbyPanel>(Main.Instance.RootPanel,delegate(Component obj) {
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
		foreach(var btn in btnOps)btn.SetActive(false);
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
			msg.Bunch.Type=pb_enum.Unknown;
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

	virtual protected IEnumerator sortHands(){yield break;}
	virtual protected bool showHints(bunch_t bunch,bool startup=false){return true;}

	protected IEnumerator passDiscard(Player player,bool wait=true){
		if(wait)yield return new WaitForSeconds(Configs.OpsInterval);
		
		//pass discard or draw
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;

		var bunch=new bunch_t();
		bunch.Pos=player.playData.Seat;
		bunch.Type=pb_enum.OpPass;
		msg.Bunch=bunch;

		Main.Instance.MainPlayer.Send<MsgCNDiscard>(msg.Mid,msg);
	}

	virtual protected IEnumerator passMeld(Player player,int card,bool wait=true){
		if(wait)yield return new WaitForSeconds(Configs.OpsInterval);
		
		//pass discard or draw
		var msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		
		bunch_t bunch=new bunch_t();
		bunch.Pos=player.playData.Seat;
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
		if(old!=Rule.Token)
			tokenIcon.Token=Rule.Token;
	}
}
