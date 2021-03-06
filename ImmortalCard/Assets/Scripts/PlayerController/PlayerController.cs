using UnityEngine;
using System.Collections;
using Google.Protobuf;
using Proto3;

public interface PlayerController {
	IEnumerator OnMsgDeal(Player player,MsgNCDeal msg);
	IEnumerator OnMsgRevive(Player player,MsgNCRevive msg);
	IEnumerator OnMsgEngage(Player player,MsgNCEngage msg);
	IEnumerator OnMsgDiscard(Player player,MsgNCDiscard msg);
	IEnumerator OnMsgMeld(Player player,MsgNCMeld msg);
	IEnumerator OnMsgDraw(Player player,MsgNCDraw msg);
	IEnumerator OnMsgSettle(Player player,MsgNCSettle msg);
	IEnumerator OnMsgFinish(Player player,MsgNCFinish msg);

	IEnumerator PreMessage(pb_msg mid,byte[] bytes);
	IEnumerator PostMessage(pb_msg mid,byte[] bytes);
}
