using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahJongPanel : GamePanel {
	public Transform[]	MeldAreas;	//MROL(Me,Right,Opposite,Left)
	public Transform[]	PlayAreas;	//MROL(Me,Right,Opposite,Left)

	public GameObject	BtnCall,BtnDouble;

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

	override public float DiscardScalar{get{return .7f;}}

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

	override protected void meld(bunch_t bunch){
		switch(bunch.Type){
		case pb_enum.BunchA:
			break;
		case pb_enum.BunchAaa:
		case pb_enum.BunchAaaa:
			break;
		default:
			break;
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
