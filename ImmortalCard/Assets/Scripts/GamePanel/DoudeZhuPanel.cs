using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuPanel : GamePanel {
	public GameObject	BtnHint;
	public GameObject	BtnDiscard,BtnCall,BtnDouble;

	override public IEnumerator OnMsgDeal(Player player,MsgNCDeal msg){
		tokenIcon.Pile=0;
		yield return StartCoroutine(base.OnMsgDeal(player,msg));

		//test only; will call Dezhu here
		//BtnCall.SetActive(true);
		//BtnDouble.SetActive(true);

		if(_pos==Rule.Banker)
			foreach(var btn in btnOps)btn.SetActive(true);
		
		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		Main.Instance.MainPlayer.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	override public IEnumerator OnMsgRevive(Player player,MsgNCRevive msg){
		yield return StartCoroutine(base.OnMsgRevive(player,msg));
		
		if(msg.Result!=pb_enum.Succeess)
			yield break;
		
		//turn off engages
		if(Main.Instance.MainPlayer.playData.SelectedCard>1000){
			BtnCall.SetActive(false);
			BtnDouble.SetActive(false);
		}
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
		}else if(_pos==pos)
			foreach(var btn in btnOps)btn.SetActive(false);
	}

	override public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield return StartCoroutine(base.OnMsgSettle(player,msg));
		
		Utils.Load<DoudeZhuSettle>(Main.Instance.RootPanel,delegate(Component obj) {
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
				bunch.Pos=player.playData.Seat;
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
			foreach(Transform ch in HandAreas[player.playData.Seat].transform){
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
		if(VerifyDiscard()){
			foreach(var btn in btnOps)btn.SetActive(false);
			StartCoroutine(Discard());
		}
	}
	
	override public void OnPass(){
		foreach(var btn in btnOps)btn.SetActive(false);
		deselectAll();
		StartCoroutine(passDiscard(Main.Instance.MainPlayer,false));
	}
	// ----------------------------------------------
	// logic
	// ----------------------------------------------
	override public bool CardDrag{get{return false;}}

	public static void Create(System.Action<Component> handler=null){
		Utils.Load<DoudeZhuPanel>(Main.Instance.RootPanel,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		});
	}
}
