using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class MahjongAIController:AIController{

	override public IEnumerator OnMsgDeal(Player player,MsgNCDeal msg){
		yield return new WaitForSeconds(Config.OpsInterval);

		var key=MahJongRule.FindDefaultColor(player);
		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=key;
		player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	override public IEnumerator OnMsgEngage(Player player,MsgNCEngage msg){
		for(int i=0;i<msg.Keys.Count;++i)
			if(player.playData.Seat==i)
				player.playData.Engagement=msg.Keys[i];
		MahJongRule.prepareAAAA(player);
		yield break;
	}

	override public IEnumerator OnMsgMeld(Player player,MsgNCMeld msg){
		//do nothing if all pass discard,because draw message will come
		if(msg.Bunch.Pos==-1&&msg.Bunch.Type==pb_enum.OpPass)
			yield break;
		
		if(player.playData.Seat==msg.Bunch.Pos){
			yield return new WaitForSeconds(Config.OpsInterval);
			if(null==Main.Instance.gameController)yield break;

			var rule=Main.Instance.gameController.Rule;
			rule.Meld(player,msg.Bunch);

			if(rule.checkDiscard(player,msg.Bunch.Pawns[0])){
				//discard
				var discard=player.playData.Hands[0];
				foreach(var hand in player.playData.Hands){
					//huazhu
					if(hand/1000==player.playData.Engagement/1000){
						discard=hand;
						break;
					}
				}

				player.unpairedCards.Add(msg.Bunch.Pawns[0]);
				
				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				omsgDiscard.Bunch=new bunch_t();
				omsgDiscard.Bunch.Pos=player.playData.Seat;
				omsgDiscard.Bunch.Pawns.Add(discard);
				omsgDiscard.Bunch.Type=pb_enum.BunchA;
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
				Debug.Log(player.playData.Seat+" discard "+discard);
			}else{
				//wait for draw
			}
		}
	}

	override public IEnumerator OnMsgDraw(Player player,MsgNCDraw msg){
		if(player.playData.Seat==msg.Pos){
			yield return new WaitForSeconds(Config.OpsInterval);
			if(null==Main.Instance.gameController)yield break;

			//meld only for the drawer
			var omsgMeld=new MsgCNMeld();
			omsgMeld.Mid=pb_msg.MsgCnMeld;
			
			bunch_t bunch=new bunch_t();
			bunch.Pos=player.playData.Seat;
			bunch.Pawns.Add(msg.Card);
			
			var hints=Main.Instance.gameController.Rule.Hint(player,bunch);
			if(hints.Count>0)
				bunch=hints[0];
			else{
				//collect
				bunch=new bunch_t();
				bunch.Pos=player.playData.Seat;
				bunch.Pawns.Add(msg.Card);
				bunch.Type=pb_enum.BunchA;
			}
			omsgMeld.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
		}
	}

	override public IEnumerator PostMessage(pb_msg mid,byte[] bytes){
		switch(mid){
		case pb_msg.MsgNcBeforeStartup:
			//MsgNcBeforeStartup msgBeforeStartup=MsgNcBeforeStartup.Parser.ParseFrom(bytes);
			break;
		default:
			break;
		}
		yield break;
	}
}