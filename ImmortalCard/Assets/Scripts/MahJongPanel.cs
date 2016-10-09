using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string CardPrefab{get{return "Mahjong";}}
	override public string Id2File(uint color,uint value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"tong","tiao","wan"};
			value=Rule.inverseTransformValue(value);
			if(color<Colors.Length)
				return string.Format("{0}{1:0}",Colors[color],value);
		}
		return "";
	}

	float AbandonScalar{get{return .7f;}}
	override public float DiscardScalar{get{return 1f;}}

	override protected bool checkDiscard(Card card=null){
		//discard my card
		var check=false;
		do{
			var token=(_token+1)%maxPlayer;
			if(token!=_pos){
				Debug.Log("Discard invalid turn");
				break;
			}
			if(null==card&&_selection.Count<=0){
				Debug.Log("Discard invalid card");
				break;
			}
			check=_historical.Count<1;
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
						curr.Pawns.Add(card.Value);
					else{
						foreach(var c in _selection)
							curr.Pawns.Add(c.Value);
					}
					if(Rule.Verify(curr,hist))
						check=true;
				}
			}
			if(!check)
				Debug.Log("Discard invalid bunch");
		}while(false);
		return check;
	}
	
	public override void TapCard(Card card,bool select=true){
		var selected=false;
		foreach(var old in _selection)if(old==card)selected=true;
		deselectAll();
		if(!selected)base.TapCard(card,select);
	}

	override protected void start(){
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

	override protected void draw(uint id,uint pos){
		//remove discards
		foreach(Transform ch in DiscardAreas[_pos].transform)Destroy(ch.gameObject);
		//discard
		float scalar=(Main.Instance.gameController==null?1f:Main.Instance.gameController.DiscardScalar);
		Card.Create(CardPrefab,id,Pile,delegate(Card card) {
			card.state=Card.State.ST_DISCARD;
			card.DiscardTo(DiscardAreas[pos],scalar);
		});
	}
	
	override protected void meld(bunch_t bunch){
		var from=_token;
		var to=bunch.Pos;
		Card A=DiscardAreas[from].GetComponentInChildren<Card>();
		if(A!=null)
		switch(bunch.Type){
		case pb_enum.BunchA:
			//collect
			A.DiscardTo(HandAreas[to],DiscardScalar);
			A.Static=false;
			A.state=Card.State.ST_NORMAL;
			if(to==_pos)sortHands();
			break;
		case pb_enum.BunchAaa:
		case pb_enum.BunchAaaa:
			//meld
			A.DiscardTo(MeldAreas[to],DiscardScalar);
			var hands=HandAreas[to].GetComponentsInChildren<Card>();
			foreach(var id in bunch.Pawns)
			foreach(var card in hands){
				if(card.Value==id)
					card.DiscardTo(MeldAreas[to],DiscardScalar);
			}
			break;
		default:
			//abandon
			A.DiscardTo(AbandonAreas[to],AbandonScalar);
			break;
		}
	}

	override protected void sortHands(){
		var hands=HandAreas[_pos].GetComponentsInChildren<Card>();
		var ids=new List<uint>();
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
	
	//List<uint[]> _hints=null;
	override protected void genHints(){
		//_hints=null;
		//_nhints=0;
		var M=HandAreas[0].childCount;
		var N=DiscardAreas[2].childCount;
		if(N>0&&M>0){
			uint[] hands=new uint[M];
			uint[] ids=new uint[N];
			int i=0;
			foreach(Transform ch in HandAreas[0].transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)hands[i++]=card.Value;
			}
			i=0;
			foreach(Transform ch in DiscardAreas[2].transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)ids[i++]=card.Value;
			}
			
			//_hints=Main.Instance.gameRule.hint(hands,ids);
		}
	}

	override protected void showHints(){
		foreach(var bunch in _hints){
			switch(bunch.Type){
			case pb_enum.BunchAaa:
				break;
			case pb_enum.BunchAaaa:
				break;
			case pb_enum.BunchWin:
				break;
			default:
				break;
			}
		}
	}
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new MahJongRule();

		var files=new List<string>();
		files.Add("dong");
		files.Add("nan");
		files.Add("xi");
		files.Add("bei");
		files.Add("zhong");
		files.Add("fa");
		files.Add("bai");
		for(uint k=1;k<=3;++k)for(uint i=1;i<=9;++i)
			files.Add(Id2File(k,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Mahjong"));
	}

	public void OnCall(){
	}

	public void OnDouble(){
	}
	
	public void OnAAA(){
		MsgCNMeld msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.BunchAaa;
		if(_hints.Count>0)
			msg.Bunch.MergeFrom(_hints[0]);
		Main.Instance.ws.Send<MsgCNMeld>(msg.Mid,msg);
	}
	
	public void OnAAAA(){
		MsgCNMeld msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.BunchAaaa;
		if(_hints.Count>0)
			msg.Bunch.MergeFrom(_hints[0]);
		Main.Instance.ws.Send<MsgCNMeld>(msg.Mid,msg);
	}
	
	public void OnWin(){
		MsgCNMeld msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.BunchWin;
		if(_hints.Count>0)
			msg.Bunch.MergeFrom(_hints[0]);
		Main.Instance.ws.Send<MsgCNMeld>(msg.Mid,msg);
	}
	
	override public void OnPass(){
		MsgCNMeld msg=new MsgCNMeld();
		msg.Mid=pb_msg.MsgCnMeld;
		msg.Bunch=new bunch_t();
		msg.Bunch.Pos=_pos;
		msg.Bunch.Type=pb_enum.OpPass;
		if(_hints.Count>0)
			msg.Bunch.MergeFrom(_hints[0]);
		Main.Instance.ws.Send<MsgCNMeld>(msg.Mid,msg);
	}
	
	public static void Create(System.Action<Component> handler=null){
		Utils.Load<MahJongPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
