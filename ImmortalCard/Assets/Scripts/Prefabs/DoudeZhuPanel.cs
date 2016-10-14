using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuPanel : GamePanel {
	public GameObject	BtnHint;
	public GameObject	BtnDiscard,BtnCall,BtnDouble;

	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public string CardPrefab{get{return "Card";}}
	override public string Id2File(int color,int value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"c","d","h","s"};
			value=Rule.inverseTransformValue(value);
			return string.Format("{0}{1:00}",Colors[color],value);
		}
		return "";
	}

	override public float DiscardScalar{get{return .625f;}}

	override protected void onMsgDiscard(MsgNCDiscard msg){
		//set to next after discard
		int pos=msg.Bunch.Pos;
		changeToken(pos+1);
		_hints=null;
		_nhints=0;

		//auto pass
		if(_pos==Rule.Token&&!showHints())
			Invoke("OnPass",Configs.OpsInterval);
	}

	int _nhints=0;
	List<bunch_t> _hints=null;
	bool showHints(){
		var player=Main.Instance.MainPlayer;
		if(_hints==null){
			bunch_t bunch=null;
			if(Rule.Historical.Count<=0){
				bunch=new bunch_t();
				bunch.Pos=player.pos;
				bunch.Type=pb_enum.OpPass;
			}else{
				bunch=Rule.Historical[Rule.Historical.Count-1];
				if(bunch.Type==pb_enum.OpPass&&Rule.Historical.Count>=2)
					bunch=Rule.Historical[Rule.Historical.Count-2];
			}
			var hands=new int[player.gameData.Hands.Count];
			player.gameData.Hands.CopyTo(hands,0);
			_hints=Rule.Hint(player,hands,bunch);
		}

		if(_hints.Count>0){
			var hints=_hints[_nhints];
			foreach(Transform ch in HandAreas[player.pos].transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)foreach(var id in hints.Pawns)
					if(card.Value==id)
						card.Tap();
			}
			_nhints=(_nhints+1)%_hints.Count;
		}
		return _hints.Count>0;
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
		for(int j=1;j<=4;++j)
			for(int i=1;i<=13;++i)
				files.Add(Id2File(j,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Card"));
	}

	public void OnHint(){
		deselectAll();
		showHints();
	}
	
	public void OnCall(){
	}

	public void OnDouble(){
	}
	
	public void OnDiscard(){
		StartCoroutine(Discard());
	}
	
	public static void Create(System.Action<Component> handler=null){
		Utils.Load<DoudeZhuPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
