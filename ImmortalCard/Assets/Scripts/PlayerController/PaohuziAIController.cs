using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class PaohuziAIController:PlayerController{
	public void onMessage(Player player,IMessage msg){
		Main.Instance.StartCoroutine(onMessageCo(player,msg));
	}

	IEnumerator onMessageCo(Player player,IMessage msg){
		if(Main.Instance.gameController==null)yield break;

		yield return new WaitForSeconds(Configs.OpsInterval);

		//var maxPlayer=Main.Instance.gameController.Rule.MaxPlayer;
		if(msg is MsgNCEngage){
			var msgEngage=msg as MsgNCEngage;
			if(player.pos==msgEngage.Pos)
				player.playData.SelectedCard=msgEngage.Key;

		}else if(msg is MsgNCStart){
			//var msgStart=msg as MsgNCStart;
			var key=MahJongRule.FindDefaultColor(player);
			var omsgEngage=new MsgCNEngage();
			omsgEngage.Mid=pb_msg.MsgCnEngage;
			omsgEngage.Key=key;
			player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);

		}else if(msg is MsgNCDiscard){
			//discard AI
			var msgDiscard=msg as MsgNCDiscard;
			var card=msgDiscard.Bunch.Pawns[0];
			if(player.pos==msgDiscard.Bunch.Pos){
				//remove from hands
				player.playData.Hands.Remove(card);
			}else{
				//meld only for others
				var omsgMeld=new MsgCNMeld();
				omsgMeld.Mid=pb_msg.MsgCnMeld;
				
				bunch_t bunch=null;
				var hints=Main.Instance.gameController.Rule.Hint(player,msgDiscard.Bunch);
				if(hints.Count>0)
					bunch=hints[0];
				else{
					bunch=new bunch_t();
					bunch.Type=pb_enum.OpPass;
					bunch.Pawns.Add(card);
				}
				bunch.Pos=player.pos;
				omsgMeld.Bunch=bunch;
				
				player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
				Debug.Log(player.pos+(hints.Count>0?(" meld "+bunch.Pawns[0]):" pass")+" after "+msgDiscard.Bunch.Pos+" discard");
			}

		}else if(msg is MsgNCMeld){
			var msgMeld=msg as MsgNCMeld;

			//do nothing if all pass discard,because draw message will come
			if(msgMeld.Bunch.Pos==-1&&msgMeld.Bunch.Type==pb_enum.OpPass)
				yield break;

			if(player.pos==msgMeld.Bunch.Pos){
				//remove from hands
				foreach(var card in msgMeld.Bunch.Pawns)
					player.playData.Hands.Remove(card);

				//discard
				var discard=player.playData.Hands[0];
				if(msgMeld.Bunch.Type==pb_enum.OpPass)
					//was draw
					discard=msgMeld.Bunch.Pawns[0];

				MsgCNDiscard omsgDiscard=new MsgCNDiscard();
				omsgDiscard.Mid=pb_msg.MsgCnDiscard;
				omsgDiscard.Bunch=new bunch_t();
				omsgDiscard.Bunch.Pos=player.pos;
				omsgDiscard.Bunch.Pawns.Add(discard);
				omsgDiscard.Bunch.Type=pb_enum.BunchA;
				player.Send<MsgCNDiscard>(omsgDiscard.Mid,omsgDiscard);
				Debug.Log(player.pos+" discard "+discard+" after self "+(int)msgMeld.Bunch.Type);
			}

		}else if(msg is MsgNCDraw){
			//already handled in Panel

		}else if(msg is MsgNCSettle){
			//var msgSettle=msg as MsgNCSettle;

		}else if(msg is MsgNCFinish){
			//var msgFinish=msg as MsgNCFinish;

		}else if(msg is MsgNCDismissSync){

		}else if(msg is MsgNCDismissAck){

		}
	}
}