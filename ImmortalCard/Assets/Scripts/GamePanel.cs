using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class GamePanel : MonoBehaviour {
	[HideInInspector]
	public int			N=3;
	public Card[]		BottomCards;

	public Transform	HandArea;
	public Transform[]	DiscardAreas;	//MRL
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;
	public GameObject	BtnHint,BtnDiscard,BtnCall,BtnDouble,BtnPass,Buttons;

	List<Card>			_selection=new List<Card>();
	uint				_pos,_banker;

	public static GamePanel	Instance=null;
	void Awake(){
		Players=new PlayerIcon[N];

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
		_banker=msg.Banker;
		//position transform
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
		hands.Sort(comparision);
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
		Debug.Log(str);
		yield break;
	}
	
	public void Discard(Card card=null){
		//remove discards
		foreach(Transform ch in DiscardAreas[0].transform)Destroy(ch.gameObject);
		//discard
		MsgCNDiscard msg=new MsgCNDiscard();
		msg.Mid=pb_msg.MsgCnDiscard;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=0;
		msg.Bunch.Type=pb_enum.BunchA;
		if(card!=null){
			deselectAll();
			card.state=Card.State.ST_DISCARD;
			card.DiscardTo(DiscardAreas[0],.625f);
			msg.Bunch.Pawns.Add(card.Value.Id);
		}else if(_selection.Count>0){
			_selection.Sort(compare_card);
			foreach(var c in _selection){
				c.state=Card.State.ST_DISCARD;
				c.DiscardTo(DiscardAreas[0],.625f);
				msg.Bunch.Pawns.Add(c.Value.Id);
			}
			_selection.Clear();
		}
		Main.Instance.ws.Send<MsgCNDiscard>(msg.Mid,msg);
	}

	public IEnumerator DiscardAt(uint pos,uint[] cards,string error=""){
		if(error.Length!=0){
			yield break;
		}
		//remove discards
		foreach(Transform ch in DiscardAreas[pos].transform)Destroy(ch.gameObject);
		if(cards!=null){
			string str="discard at "+pos;
			for(int i=0;i<cards.Length;++i){
				var id=cards[i];
				var v=Configs.Cards[id];
				var fin=false;
				Card.Create(v,nHandCards[1].transform.parent,delegate(Card card) {
					card.state=Card.State.ST_DISCARD;
					card.DiscardTo(DiscardAreas[1],.625f);
					fin=true;
				});
				yield return null;
				str+="("+v.Id+","+v.Color+","+v.Value+"),";
				while(!fin)yield return null;
			}
			//Debug.Log(str);
		}
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
	}
	
	public void OnDiscard(){
		Discard();
	}

	public void OnPass(){
		deselectAll();
		uint[] cards=new uint[2];
		for(int i=0;i<cards.Length;++i){
			var hands=Main.Instance.gameRule.Hands[0];
			if(hands.Count>0){
				cards[i]=hands[0];
				hands.RemoveAt(0);
			}
		}
		StartCoroutine(DiscardAt(0,cards));
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
		return comparision(x.Value.Id,y.Value.Id);
	}

	int comparision(uint x,uint y){
		var cx=Configs.Cards[x];
		var cy=Configs.Cards[y];
		if(cx.Value==1||cx.Value==2)
			x+=(53-2);
		else if(cx.Value==14)
			x+=8;
		if(cy.Value==1||cy.Value==2)
			y+=(53-2);
		else if(cy.Value==14)
			y+=8;
		
		if(x>y)
			return -1;
		else if(x<y)
			return 1;
		return 0;
	}
}
