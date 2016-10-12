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
	override public string CardPrefab{get{return "Card";}}
	override public string Id2File(uint color,uint value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"c","d","h","s"};
			value=Rule.inverseTransformValue(value);
			return string.Format("{0}{1:00}",Colors[color],value);
		}
		return "";
	}

	override public float DiscardScalar{get{return .625f;}}

	override protected bool checkDiscard(Card card=null){
		//discard my card
		var check=false;
		do{
			if(_pos!=_token%maxPlayer){
				Debug.Log("Discard invalid turn");
				break;
			}
			if(null==card&&_selection.Count<=0){
				Debug.Log("Discard invalid card");
				break;
			}

			check=Rule.Historical.Count<1;
			if(!check){
				var hist=Rule.Historical[Rule.Historical.Count-1];
				if(hist.Type==pb_enum.OpPass&&Rule.Historical.Count>=2)
					hist=Rule.Historical[Rule.Historical.Count-2];
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
					if(rule.Verify(curr,hist))
						check=true;
				}
			}
			if(!check)
				Debug.Log("Discard invalid bunch");
		}while(false);
		return check;
	}

	override protected void discard(uint pos){
		//set to next after discard
		changeToken(pos+1);
	}

	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();
		Rule=new DoudeZhuRule();

		var files=new List<string>();
		files.Add("back");
		files.Add("c14");
		files.Add("d15");
		for(uint j=1;j<=4;++j)
			for(uint i=1;i<=13;++i)
				files.Add(Id2File(j,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Card"));
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
