using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PaohuziAIController:AIController{

	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		PaohuziRule.prepareAAAA(player);
		yield break;
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		var rule=Main.Instance.gameController.Rule;
		var card=msg.Bunch.Pawns[0];
		var bDraw=rule.Pile.IndexOf(card)!=-1;

		//do nothing if all pass discard,because draw message will come
		if(msg.Bunch.Type==pb_enum.OpPass && !bDraw && card!=Config.invalidCard)
			yield break;
		
		if(player.playData.Seat==msg.Bunch.Pos){
			yield return new WaitForSeconds(Config.OpsInterval);
			if(null==Main.Instance.gameController)yield break;

			rule.Meld(player,msg.Bunch);

			if(msg.Bunch.Type==pb_enum.BunchWin)
				yield break;

			if(rule.checkDiscard(player,Config.invalidCard)){
				//discard
				var discard=player.playData.Hands[0];

				player.unpairedCards.Add(card);
				
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				omsgDiscard.Bunch=new bunch_t();
				omsgDiscard.Bunch.Pos=player.playData.Seat;
				omsgDiscard.Bunch.Pawns.Add(discard);
				omsgDiscard.Bunch.Type=bDraw?pb_enum.BunchA:pb_enum.Unknown;
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);

				Debug.Log(player.playData.Seat+" discard "+discard);
			}else{
				//wait for next draw
			}
		}
	}
}