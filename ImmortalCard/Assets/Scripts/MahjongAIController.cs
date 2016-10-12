using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class MahjongAIController:PlayerController{
	public void onMessage(Player player,IMessage msg){
		if(Main.Instance.gameController==null)return;
		//var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(msg is MsgNCStart){
			//var msgStart=msg as MsgNCStart;

		}else if(msg is MsgNCDiscard){
			//discard AI
			var msgDiscard=msg as MsgNCDiscard;
			if(player.pos==msgDiscard.Bunch.Pos){
				//remove from hands
				foreach(var card in msgDiscard.Bunch.Pawns)
					player.gameData.Hands.Remove(card);
			}else{
				//meld
				var omsgMeld=new MsgCNMeld();
				omsgMeld.Mid=pb_msg.MsgCnMeld;
				
				bunch_t bunch=null;
				var hands=new uint[player.gameData.Hands.Count];
				player.gameData.Hands.CopyTo(hands,0);
				var hints=Main.Instance.gameController.Rule.Hint(player,hands,msgDiscard.Bunch);
				if(hints.Count>0)
					bunch=hints[0];
				else{
					bunch=new bunch_t();
					bunch.Pos=player.pos;
					bunch.Type=pb_enum.OpPass;
				}
				omsgMeld.Bunch=bunch;
				
				player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
			}

		}else if(msg is MsgNCMeld){
			var msgMeld=msg as MsgNCMeld;
			if(player.pos==msgMeld.Bunch.Pos){
				//remove from hands
				foreach(var card in msgMeld.Bunch.Pawns)
					player.gameData.Hands.Remove(card);
			}

		}else if(msg is MsgNCDraw){
			var msgDraw=msg as MsgNCDraw;
			//meld
			var omsgMeld=new MsgCNMeld();
			omsgMeld.Mid=pb_msg.MsgCnMeld;
			
			bunch_t bunch=new bunch_t();
			bunch.Pos=player.pos;
			bunch.Pawns.Add(msgDraw.Card);

			var hands=new uint[player.gameData.Hands.Count];
			player.gameData.Hands.CopyTo(hands,0);
			var hints=Main.Instance.gameController.Rule.Hint(player,hands,bunch);
			if(hints.Count>0)
				bunch=hints[0];
			else{
				bunch=new bunch_t();
				bunch.Pos=player.pos;
				bunch.Type=pb_enum.OpPass;
			}
			omsgMeld.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);

		}else if(msg is MsgNCSettle){
			//var msgSettle=msg as MsgNCSettle;

		}else if(msg is MsgNCFinish){
			//var msgFinish=msg as MsgNCFinish;

		}else if(msg is MsgNCDismissSync){

		}else if(msg is MsgNCDismissAck){

		}
	}
}