using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class DoudeZhuAIController:PlayerController {
	public void onMessage(Player player,IMessage msg){
		if(Main.Instance.gameController==null)return;
		var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(msg is MsgNCStart){
			//var msgStart=msg as MsgNCStart;

		}else if(msg is MsgNCDiscard){
			//discard AI
			var msgDiscard=msg as MsgNCDiscard;
			if(player.pos==msgDiscard.Bunch.Pos){
				foreach(var card in msgDiscard.Bunch.Pawns)
					player.gameData.Hands.Remove(card);
			}else if(player.pos==(msgDiscard.Bunch.Pos+1)%maxPlayer){
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				
				bunch_t bunch=null;
				var hands=new int[player.gameData.Hands.Count];
				player.gameData.Hands.CopyTo(hands,0);
				var hints=Main.Instance.gameController.Rule.Hint(player,hands,msgDiscard.Bunch);
				if(hints.Count>0)
					bunch=hints[0];
				else{
					bunch=new bunch_t();
					bunch.Pos=player.pos;
					bunch.Type=pb_enum.OpPass;
				}
				omsgDiscard.Bunch=bunch;
				
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
