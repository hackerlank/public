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
	override public bool CardDrag{get{return false;}}
	override public string CardPrefab{get{return "Card";}}
	override public string Id2File(int color,int value){
		if(Rule!=null){
			color-=1;
			string[] Colors={"c","d","h","s"};
			value=Rule.inverseTransformValue(value);
			return CardPrefab+"/"+string.Format("{0}{1:00}",Colors[color],value);
		}
		return "";
	}

	override public float DiscardScalar{get{return .625f;}}

	override public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		yield return StartCoroutine(base.OnMsgStart(player,msg));
		BtnCall.SetActive(false);
		BtnDouble.SetActive(false);
		foreach(var btn in btnOps)btn.SetActive(true);

		//test only; will call Dezhu here
		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	override public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		yield return StartCoroutine(base.OnMsgDiscard(player,msg));
		//set to next after discard
		int pos=msg.Bunch.Pos;
		changeToken(pos+1);
		_hints=null;
		_nhints=0;

		//auto pass
		if(_pos==Rule.Token){
			if(showHints())
				foreach(var btn in btnOps)btn.SetActive(true);
			else{
				foreach(var btn in btnOps)btn.SetActive(false);
				StartCoroutine(passDiscard(Main.Instance.MainPlayer));
			}
		}
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));
		
		Utils.Load<DoudeZhuSettle>(Main.Instance.transform,delegate(Component obj) {
			var popup=obj as SettlePopup;
			popup.Value=msg;
		});
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
			_hints=Rule.Hint(player,bunch);
		}

		if(_hints.Count>0){
			var hints=_hints[_nhints];
			_selection.Clear();
			foreach(Transform ch in HandAreas[player.pos].transform){
				var card=ch.gameObject.GetComponent<Card>();
				if(card!=null)foreach(var id in hints.Pawns)
				if(card.Value==id){
					card.Tap();
					_selection.Add(card);
				}
			}
			_nhints=(_nhints+1)%_hints.Count;
		}
		return _hints.Count>0;
	}

	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		Rule=new DoudeZhuRule();
		base.Awake();

		var files=new List<string>();
		files.Add(CardPrefab+"/"+"back");
		files.Add(CardPrefab+"/"+"c14");
		files.Add(CardPrefab+"/"+"d15");
		for(int j=1;j<=4;++j)
			for(int i=1;i<=13;++i)
				files.Add(Id2File(j,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Card"));

		btnOps=new GameObject[]{BtnHint,BtnDiscard,BtnPass};
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
		foreach(var btn in btnOps)btn.SetActive(false);
		StartCoroutine(Discard());
	}
	
	override public void OnPass(){
		foreach(var btn in btnOps)btn.SetActive(false);
		deselectAll();
		StartCoroutine(passDiscard(Main.Instance.MainPlayer,false));
	}

	public static void Create(System.Action<Component> handler=null){
		Utils.Load<DoudeZhuPanel>(Main.Instance.transform,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
