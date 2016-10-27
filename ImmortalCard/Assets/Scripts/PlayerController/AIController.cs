using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class AIController:PlayerController{

	virtual protected void onMsgStart(Player player,MsgNCStart msg){
		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=0;
		player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	virtual protected void onMsgEngage(Player player,MsgNCEngage msg){}

	virtual protected void onMsgDiscard(Player player,MsgNCDiscard msg){
		//discard AI
		var card=msg.Bunch.Pawns[0];
		if(player.pos==msg.Bunch.Pos){
			//remove from hands
			player.playData.Hands.Remove(card);
		}else{
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
			bunch.Pos=player.pos;
			omsgMeld.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
			Debug.Log(player.pos+(hints.Count>0?(" meld "+bunch.Pawns[0]):" pass")+" after "+msg.Bunch.Pos+" discard");
		}
	}

	virtual protected void onMsgMeld(Player player,MsgNCMeld msg){}

	virtual protected void onMsgDraw(Player player,MsgNCDraw msg){}

	public IEnumerator onMessage(Player player,IMessage msg){
		if(Main.Instance.gameController==null)yield break;
		
		yield return new WaitForSeconds(Configs.OpsInterval);
		
		//var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(msg is MsgNCStart){
			onMsgStart(player,msg as MsgNCStart);

		}else if(msg is MsgNCEngage){
			onMsgEngage(player,msg as MsgNCEngage);

		}else if(msg is MsgNCDiscard){
			onMsgDiscard(player,msg as MsgNCDiscard);

		}else if(msg is MsgNCMeld){
			onMsgMeld(player,msg as MsgNCMeld);

		}else if(msg is MsgNCDraw){
			onMsgDraw(player,msg as MsgNCDraw);
			
		}else if(msg is MsgNCSettle){
			//var msgSettle=msg as MsgNCSettle;
			
		}else if(msg is MsgNCFinish){
			//var msgFinish=msg as MsgNCFinish;
			
		}else if(msg is MsgNCDismissSync){
			
		}else if(msg is MsgNCDismissAck){
			
		}
	}
}
