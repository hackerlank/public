using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PlayerAIController {
	static public void onMessage(Player player,IMessage msg){
		if(Main.Instance.gameController==null)return;
		var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		Loom.QueueOnMainThread(delegate{
			if(msg is MsgNCStart){
				var msgStart=msg as MsgNCStart;
				player.pos=msgStart.Pos;
				player.gameData.Hands.AddRange(msgStart.Hands);
			}else if(msg is MsgNCDiscard){
				//simplest discard AI
				var msgDiscard=msg as MsgNCDiscard;
				if(player.pos==(msgDiscard.Bunch.Pos+1)%maxPlayer){
					var card=msgDiscard.Bunch.Pawns[0];
					uint discard=0;
					foreach(var id in player.gameData.Hands){
						if(id%100>card%100){
							player.gameData.Hands.Remove(id);
							discard=id;
						}
					}
					MsgCNDiscard omsgDiscard=new MsgCNDiscard();
					omsgDiscard.Mid=pb_msg.MsgCnDiscard;
					omsgDiscard.Bunch=new bunch_t();
					omsgDiscard.Bunch.Pos=player.pos;
					omsgDiscard.Bunch.Pawns.Add(discard);
					omsgDiscard.Bunch.Type=(discard==0?pb_enum.OpPass:pb_enum.BunchA);
					player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
				}
			}else if(msg is MsgNCMeld){
				var msgMeld=msg as MsgNCMeld;
			}else if(msg is MsgNCDraw){
				var msgDraw=msg as MsgNCDraw;
			}else if(msg is MsgNCSettle){
				var msgSettle=msg as MsgNCSettle;
			}else if(msg is MsgNCFinish){
				var msgFinish=msg as MsgNCFinish;
			}else if(msg is MsgNCDismissSync){
			}else if(msg is MsgNCDismissAck){
			}
		});
	}
}
