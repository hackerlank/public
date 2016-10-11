using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PlayerController {
	static public void onMessage(Player player,IMessage msg){
		Loom.QueueOnMainThread(delegate{
			var controller=Main.Instance.gameController;
			if(controller!=null){
				if(msg is MsgNCStart){
					var msgStart=msg as MsgNCStart;
					Main.Instance.StartCoroutine(controller.OnMsgStart(msgStart));
				}else if(msg is MsgNCDiscard){
					var msgDiscard=msg as MsgNCDiscard;
					Main.Instance.StartCoroutine(controller.OnMsgDiscard(msgDiscard));
				}else if(msg is MsgNCMeld){
					var msgMeld=msg as MsgNCMeld;
					controller.OnMsgMeld(msgMeld);
				}else if(msg is MsgNCDraw){
					var msgDraw=msg as MsgNCDraw;
					controller.OnMsgDraw(msgDraw);
				}else if(msg is MsgNCSettle){
					var msgSettle=msg as MsgNCSettle;
					controller.OnMsgSettle(msgSettle);
				}else if(msg is MsgNCFinish){
					var msgFinish=msg as MsgNCFinish;
					controller.OnMsgFinish(msgFinish);
				}else if(msg is MsgNCDismissSync){
				}else if(msg is MsgNCDismissAck){
				}
			}
		});
	}
}
