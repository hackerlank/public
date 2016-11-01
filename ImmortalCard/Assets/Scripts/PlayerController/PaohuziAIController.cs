using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PaohuziAIController:AIController{

	override protected void onMsgEngage(Player player,MsgNCEngage msg){
		PaohuziRule.prepareAAAA(player);
	}

	override protected void onMsgMeld(Player player,MsgNCMeld msg){
		//do nothing if all pass discard,because draw message will come
		if(msg.Bunch.Pos==-1&&msg.Bunch.Type==pb_enum.OpPass)
			return;
		
		if(player.pos==msg.Bunch.Pos){
			var rule=Main.Instance.gameController.Rule;
			rule.Meld(player,msg.Bunch);

			if(rule.checkDiscard(player)){
				//discard
				var discard=player.playData.Hands[0];
				if(msg.Bunch.Type==pb_enum.OpPass)
					//was draw
					discard=msg.Bunch.Pawns[0];
				
				player.unpairedCards.Add(msg.Bunch.Pawns[0]);
				
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				omsgDiscard.Bunch=new bunch_t();
				omsgDiscard.Bunch.Pos=player.pos;
				omsgDiscard.Bunch.Pawns.Add(discard);
				omsgDiscard.Bunch.Type=pb_enum.BunchA;
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
				Debug.Log(player.pos+" discard "+discard+" after self "+(int)msg.Bunch.Type);
			}else{
				//next draw
			}
		}
	}
}