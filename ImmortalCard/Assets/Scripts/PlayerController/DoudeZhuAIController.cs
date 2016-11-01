using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class DoudeZhuAIController:AIController {
	override protected void onMsgDiscard(Player player,MsgNCDiscard msg){
		//discard AI
		var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(player.pos==(msg.Bunch.Pos+1)%maxPlayer){
			var rule=Main.Instance.gameController.Rule;
			if(rule.checkDiscard(player,0)){
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				
				bunch_t bunch=null;
				var hints=rule.Hint(player,msg.Bunch);
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
		}
	}
}
