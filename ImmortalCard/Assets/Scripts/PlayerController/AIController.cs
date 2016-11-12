using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class AIController:PlayerController{

	virtual public IEnumerator OnMsgStart(Player player,MsgNCStart msg){
		yield return new WaitForSeconds(Configs.OpsInterval);

		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	virtual public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		yield break;
	}

	virtual public IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg){
		//should execute immediately before remove card from hands
		if(Main.Instance.MainPlayer==player)yield break;

		var fromSelf=false;
		var card=msg.Bunch.Pawns[0];
		foreach(var h in player.playData.Hands)if(h==card){fromSelf=true;break;}

		if(fromSelf){
			//remove from hands
			player.playData.Hands.Remove(card);
		}else{
			yield return new WaitForSeconds(Configs.OpsInterval);
			if(Main.Instance.gameController==null)yield break;

			//meld only for others
			var omsgMeld=new MsgCNMeld();
			omsgMeld.Mid=pb_msg.MsgCnMeld;
			
			bunch_t bunch=null;
			var hints=Main.Instance.gameController.Rule.Hint(player,msg.Bunch);
			if(hints.Count>0)
				bunch=hints[0];
			else{
				bunch=new bunch_t();
				bunch.Type=pb_enum.OpPass;
				bunch.Pawns.Add(card);
			}
			if(bunch.Type==pb_enum.PhzAbc){
				//packed bunch
				if(bunch.Child.Count>0){
					var child=bunch.Child[0];
					foreach(var ch0 in child.Pawns)bunch.Pawns.Add(ch0);
					if(child.Child.Count>0){
						child=child.Child[0];
						foreach(var ch1 in child.Pawns)bunch.Pawns.Add(ch1);
					}
				}
			}
			bunch.Pos=player.playData.Seat;
			omsgMeld.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
			//Debug.Log(player.pos+(hints.Count>0?(" meld "+bunch.Pawns[0]):" pass")+" after "+msg.Bunch.Pos+" discard");
		}
	}
	
	virtual public IEnumerator OnMsgDraw(Player player,MsgNCDraw msg){
		yield break;
	}

	virtual public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		yield break;
	}

	virtual public IEnumerator OnMsgSettle(Player player,MsgNCSettle msg){
		yield break;
	}
	
	virtual public IEnumerator OnMsgFinish(Player player,MsgNCFinish msg){
		yield break;
	}
}
