using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuPanel : GamePanel {
	public GameObject	BtnCall,BtnDouble;

	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string Id2File(uint color,uint value){
		if(Rule!=null){
			string[] Colors={"c","d","h","s"};
			value=Rule.inverseTransformValue(value);
			return string.Format("{0}{1:00}",Colors[color],value);
		}
		return "";
	}

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
						curr.Pawns.Add(card.Value.Id);
					else{
						foreach(var c in _selection)
							curr.Pawns.Add(c.Value.Id);
					}
					if(rule.Verify(curr,hist))
						check=true;
				}
			}
			if(!check)
				Debug.Log("Discard invalid bunch");
		}while(false);
		return check;
	}
	
	//List<uint[]> _hints=null;
	override protected void hint(){
		//_hints=null;
		//_nhints=0;
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
			
			//_hints=Main.Instance.gameRule.hint(hands,ids);
		}
	}

	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new DoudeZhuRule();
	}

	public void OnCall(){
	}

	public void OnDouble(){
	}
	
	public static void Create(System.Action<Component> handler=null){
		Utils.Load<DoudeZhuPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
