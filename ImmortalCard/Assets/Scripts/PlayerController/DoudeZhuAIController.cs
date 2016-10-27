using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class DoudeZhuAIController:PlayerController {
	public void onMessage(Player player,IMessage msg){
		Main.Instance.StartCoroutine(onMessageCo(player,msg));
	}
	
	IEnumerator onMessageCo(Player player,IMessage msg){
		if(Main.Instance.gameController==null)yield break;

		var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(msg is MsgNCStart){
			//var msgStart=msg as MsgNCStart;
			var omsgEngage=new MsgCNEngage();
			omsgEngage.Mid=pb_msg.MsgCnEngage;
			omsgEngage.Key=0;
			player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
			
		}else if(msg is MsgNCDiscard){
			//discard AI
			var msgDiscard=msg as MsgNCDiscard;
			if(player.pos==(msgDiscard.Bunch.Pos+1)%maxPlayer){
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				
				bunch_t bunch=null;
				var hints=Main.Instance.gameController.Rule.Hint(player,msgDiscard.Bunch);
				if(hints.Count>0)
					bunch=hints[0];
				else{
					bunch=new bunch_t();
					bunch.Pos=player.pos;
					bunch.Type=pb_enum.OpPass;
				}
				omsgDiscard.Bunch=bunch;
				//Debug.Log(player.pos+" discard "+Player.bunch2str(omsgDiscard.Bunch));
				
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
			}

		}else if(msg is MsgNCSettle){
			//var msgSettle=msg as MsgNCSettle;

		}else if(msg is MsgNCFinish){
			//var msgFinish=msg as MsgNCFinish;

		}else if(msg is MsgNCDismissSync){

		}else if(msg is MsgNCDismissAck){

		}
	}
}
