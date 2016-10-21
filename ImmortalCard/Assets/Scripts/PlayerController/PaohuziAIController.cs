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
				player.gameData.SelectedCard=msgEngage.Key;

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
			if(player.pos==msgDiscard.Bunch.Pos){
				//remove from hands
				foreach(var card in msgDiscard.Bunch.Pawns)
					player.gameData.Hands.Remove(card);
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
					bunch.Pos=player.pos;
					bunch.Type=pb_enum.OpPass;
				}
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
					player.gameData.Hands.Remove(card);

				//discard
				var discard=player.gameData.Hands[0];
				foreach(var hand in player.gameData.Hands){
					//huazhu
					if(hand/1000==player.gameData.SelectedCard/1000){
						discard=hand;
						break;
					}
				}
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
			var msgDraw=msg as MsgNCDraw;
			if(player.pos==msgDraw.Pos){
				//meld only for the drawer
				var omsgMeld=new MsgCNMeld();
				omsgMeld.Mid=pb_msg.MsgCnMeld;
				
				bunch_t bunch=new bunch_t();
				bunch.Pos=player.pos;
				bunch.Pawns.Add(msgDraw.Card);

				var hints=Main.Instance.gameController.Rule.Hint(player,bunch);
				if(hints.Count>0)
					bunch=hints[0];
				else{
					bunch=new bunch_t();
					bunch.Pos=player.pos;
					bunch.Type=pb_enum.OpPass;
				}
				omsgMeld.Bunch=bunch;
				
				player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
				Debug.Log(player.pos+(hints.Count>0?(" meld "+bunch.Pawns[0]):" pass")+" after self draw");
			}
			
		}else if(msg is MsgNCSettle){
			//var msgSettle=msg as MsgNCSettle;

		}else if(msg is MsgNCFinish){
			//var msgFinish=msg as MsgNCFinish;

		}else if(msg is MsgNCDismissSync){

		}else if(msg is MsgNCDismissAck){

		}
	}
}