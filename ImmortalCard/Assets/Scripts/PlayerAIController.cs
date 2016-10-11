using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PlayerAIController {
	static public void onMessage(Player player,IMessage msg){
		Loom.QueueOnMainThread(delegate{
			if(msg is MsgNCStart){
				var msgStart=msg as MsgNCStart;
				player.pos=msgStart.Pos;
				player.gameData.Hands.AddRange(msgStart.Hands);
			}else if(msg is MsgNCDiscard){
				var msgDiscard=msg as MsgNCDiscard;
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
