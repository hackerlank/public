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
		//do nothing if all pass discard,because draw message will come
		if(msg.Bunch.Pos==-1&&msg.Bunch.Type==pb_enum.OpPass)
			yield break;
		
		if(player.pos==msg.Bunch.Pos){
			yield return Main.Instance.StartCoroutine(base.OnMsgMeld(player,msg));

			var rule=Main.Instance.gameController.Rule;
			rule.Meld(player,msg.Bunch);

			var bDraw=(msg.Bunch.Type==pb_enum.OpPass);
			//var bDraw=rule.Pile.IndexOf(msg.Bunch.Pawns[0])!=-1;
			if(rule.checkDiscard(player,bDraw?msg.Bunch.Pawns[0]:Configs.invalidCard)){
				//discard
				var discard=player.playData.Hands[0];
				if(bDraw)
					//was draw
					discard=msg.Bunch.Pawns[0];
				
				player.unpairedCards.Add(msg.Bunch.Pawns[0]);
				
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				omsgDiscard.Bunch=new bunch_t();
				omsgDiscard.Bunch.Pos=player.pos;
				omsgDiscard.Bunch.Pawns.Add(discard);
				omsgDiscard.Bunch.Type=bDraw?pb_enum.BunchA:pb_enum.Unknown;
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
			}else{
				//next draw
			}
		}
	}
}