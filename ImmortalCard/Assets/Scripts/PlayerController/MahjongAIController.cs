﻿using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class MahjongAIController:AIController{

	override protected void onMsgStart(Player player,MsgNCStart msg){
		var key=MahJongRule.FindDefaultColor(player);
		var omsgEngage=new MsgCNEngage();
		omsgEngage.Mid=pb_msg.MsgCnEngage;
		omsgEngage.Key=key;
		player.Send<MsgCNEngage>(omsgEngage.Mid,omsgEngage);
	}

	override protected void onMsgEngage(Player player,MsgNCEngage msg){
		for(int i=0;i<msg.Keys.Count;++i)
			if(player.pos==i)
				player.playData.SelectedCard=msg.Keys[i];
	}

	override protected void onMsgMeld(Player player,MsgNCMeld msg){
		//do nothing if all pass discard,because draw message will come
		if(msg.Bunch.Pos==-1&&msg.Bunch.Type==pb_enum.OpPass)
			return;
		
		if(player.pos==msg.Bunch.Pos){
			Main.Instance.gameController.Rule.Meld(player,msg.Bunch);
			
			//discard
			var discard=player.playData.Hands[0];
			foreach(var hand in player.playData.Hands){
				//huazhu
				if(hand/1000==player.playData.SelectedCard/1000){
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
			Debug.Log(player.pos+" discard "+discard+" after self "+(int)msg.Bunch.Type);
		}
	}

	override protected void onMsgDraw(Player player,MsgNCDraw msg){
		if(player.pos==msg.Pos){
			//meld only for the drawer
			var omsgMeld=new MsgCNMeld();
			omsgMeld.Mid=pb_msg.MsgCnMeld;
			
			bunch_t bunch=new bunch_t();
			bunch.Pos=player.pos;
			bunch.Pawns.Add(msg.Card);
			
			var hints=Main.Instance.gameController.Rule.Hint(player,bunch);
			if(hints.Count>0)
				bunch=hints[0];
			else{
				bunch=new bunch_t();
				bunch.Pos=player.pos;
				bunch.Pawns.Add(msg.Card);
				bunch.Type=pb_enum.OpPass;
			}
			omsgMeld.Bunch=bunch;
			
			player.Send<MsgCNMeld>(omsgMeld.Mid,omsgMeld);
			Debug.Log(player.pos+(hints.Count>0?(" meld "+bunch.Pawns[0]):" pass")+" after self draw");
		}
	}
}