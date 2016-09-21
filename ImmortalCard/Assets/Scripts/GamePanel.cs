using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class GamePanel : MonoBehaviour {
	[HideInInspector]
	public uint			N=3;
	public Card[]		BottomCards;

	public Transform	HandArea;
	public Transform[]	DiscardAreas;	//MRL
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;
	public GameObject	BtnHint,BtnDiscard,BtnCall,BtnDouble,BtnPass,Buttons;

	List<Card>			_selection;
	uint				_pos,_token,_banker;
	List<bunch_t>		_historical;

	public static GamePanel	Instance=null;
	void Awake(){
		Instance=this;
	}

	IEnumerator Start(){
		while(!CardCache.Ready)yield return null;
	}

	void OnDestroy(){
		Instance=null;
	}

	public IEnumerator Deal(MsgNCStart msg){
		_pos=msg.Pos;
		_token=(msg.Banker+N-1)%N;	//set to the previous position
		_banker=msg.Banker;
		_historical=new List<bunch_t>();
		_selection=new List<Card>();
		/* position transform
		(R)          (L)
			(M=_pos)
		*/
		var M=_pos;
		var R=(M+1)%N;
		var L=(M+2)%N;
		Transform[] tempD=new Transform[DiscardAreas.Length];	//MRL
		PlayerIcon[] tempP=new PlayerIcon[Players.Length];
		Text[] tempH=new Text[nHandCards.Length];
		DiscardAreas.CopyTo(tempD,0);
		Players.CopyTo(tempP,0);
		nHandCards.CopyTo(tempH,0);
		DiscardAreas[0]=tempD[M];
		DiscardAreas[1]=tempD[R];
		DiscardAreas[2]=tempD[L];
		Players[0]=tempP[M];
		Players[1]=tempP[R];
		Players[2]=tempP[L];
		nHandCards[0]=tempH[M];
		nHandCards[1]=tempH[R];
		nHandCards[2]=tempH[L];

		//dict
		Configs.Cards=new Dictionary<uint, pawn_t>();
		for(int i=0;i<msg.Cards.Count;++i){
			var card=msg.Cards[i];
			Configs.Cards[card.Id]=card;
		}
		//sort
		var hands=new List<uint>(msg.Hands);
		hands.Sort(Main.Instance.gameRule.comparision);
		//deal
		string str="deal: banker="+msg.Banker+"pos="+msg.Pos+"\nhands:\n";
		for(int i=0;i<hands.Count;++i){
			var id=hands[i];
			var v=Configs.Cards[id];
			var fin=false;
			Card.Create(v,HandArea,delegate(Card card) {
				card.Static=false;
				fin=true;
			});
			yield return null;
			str+="("+v.Id+","+v.Color+","+v.Value+"),";
			if((i+1)%6==0)str+="\n";
			while(!fin)yield return null;
		}
		Ante.text=string.Format("Ante: {0}",msg.Ante);
		Multiples.text=string.Format("Multiple: {0}",msg.Multiple);
		for(int i=0;i<msg.Count.Count;++i)
			nHandCards[i].text=msg.Count[i].ToString();
		for(int i=0;i<msg.Bottom.Count;++i){
			var v=Configs.Cards[msg.Bottom[i]];
			BottomCards[i].Value=v;
		}
		if(Players[_banker].gameTimer!=null)
			Players[_banker].gameTimer.On();
		Debug.Log(str);
		yield break;
	}
	
	public void Discard(Card card=null){
		//discard my card
		do{
			var token=(_token+1)%N;
			if(token!=_pos){
				Debug.Log("Discard invalid turn");
				break;
			}
			if(null==card&&_selection.Count<=0){
				Debug.Log("Discard invalid card");
				break;
			}
			var check=_historical.Count<1;
			if(!check){
				var hist=_historical[_historical.Count-1];
				if(hist.Type==pb_enum.OpPass&&_historical.Count>=2)
					hist=_historical[_historical.Count-2];
				if(hist.Type==pb_enum.OpPass)
					check=true;
				else{
					bunch_t curr=new bunch_t();
					curr.Pos=_pos;
					if(card!=null)
						curr.Pawns.Add(card.Value.Id);
					else{
						foreach(var c in _selection)
							curr.Pawns.Add(c.Value.Id);
					}
					if(Main.Instance.gameRule.Verify(curr,hist))
						check=true;
				}
			}
			if(!check){
				Debug.Log("Discard invalid bunch");
				break;
			}
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
				card.DiscardTo(DiscardAreas[_pos],.625f);
				msg.Bunch.Pawns.Add(card.Value.Id);
			}else if(_selection.Count>0){
				_selection.Sort(compare_card);
				foreach(var c in _selection){
					c.state=Card.State.ST_DISCARD;
					c.DiscardTo(DiscardAreas[_pos],.625f);
					msg.Bunch.Pawns.Add(c.Value.Id);
				}
				_selection.Clear();
			}
			Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
		}while(false);
	}

	public IEnumerator OnDiscardAt(MsgNCDiscard msg){
		//discard any body's card
		if(msg.Result!=pb_enum.Succeess){
			yield break;
		}

		_token=msg.Bunch.Pos;
		string str="discard at "+_token;
		var cards=new uint[msg.Bunch.Pawns.Count];
		msg.Bunch.Pawns.CopyTo(cards,0);
		if(_token==_pos){
			//adjust by feedback
		}else{
			//remove discards
			foreach(Transform ch in DiscardAreas[_token].transform)Destroy(ch.gameObject);
			for(int i=0;i<cards.Length;++i){
				var id=cards[i];
				var v=Configs.Cards[id];
				var fin=false;
				Card.Create(v,nHandCards[_token].transform.parent,delegate(Card card) {
					card.state=Card.State.ST_DISCARD;
					card.DiscardTo(DiscardAreas[_token],.625f);
					fin=true;
				});
				yield return null;
				str+="("+v.Id+","+v.Color+","+v.Value+"),";
				while(!fin)yield return null;
			}
		}
		//record
		var nCards=int.Parse(nHandCards[_token].text)-1;
		nHandCards[_token].text=nCards.ToString();
		_historical.Add(msg.Bunch);
		next(_token);
		//Debug.Log(str);
	}

	List<uint[]> _hints=null;
	public void Hint(){
		_hints=null;
		_nhints=0;
		var M=HandArea.childCount;
		var N=DiscardAreas[2].childCount;
		if(N>0&&M>0){
			uint[] hands=new uint[M];
			uint[] ids=new uint[N];
			int i=0;
			foreach(Transform ch in HandArea.transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)hands[i++]=card.Value.Id;
			}
			i=0;
			foreach(Transform ch in DiscardAreas[2].transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)ids[i++]=card.Value.Id;
			}
			
			_hints=Main.Instance.gameRule.Hint(hands,ids);
		}
	}

	void deselectAll(){
		var copy=new List<Card>(_selection);
		foreach(var c in copy)c.Tap();
	}

	public void OnCard(Card card,bool select=true){
		if(select)
			_selection.Add(card);
		else
			_selection.Remove(card);
	}

	int _nhints=0;
	public void OnHint(){
		deselectAll();
		/*
		if(_hints==null)Hint();

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
	
	public void OnDiscard(){
		Discard();
	}

	public void OnPass(){
		deselectAll();
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.OpPass;
		Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
	}
	
	public void OnCall(){
	}

	public void OnDouble(){
	}
	
	public void OnExit(){
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}

	public static void Create(System.Action<Component> handler=null){
		Utils.Load<GamePanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}

	//System.Comparison<uint>
	int compare_card(Card x,Card y){
		return Main.Instance.gameRule.comparision(x.Value.Id,y.Value.Id);
	}

	void next(uint pos){
		var R=(pos+1)%N;
		if(Players[pos].gameTimer!=null)
			Players[pos].gameTimer.On(false);
		if(Players[R].gameTimer!=null)
			Players[R].gameTimer.On();
	}
}
