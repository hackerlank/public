using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public class Player {
	//networking
	public delegate void	MessageHandler(pb_msg mid,byte[] bytes);
	public HttpProxy		http;
	public play_t			playData=new play_t();

	//phz
	public MsgLCLogin		msgLCLogin;

	public Player(){
		//networks
		http=new HttpProxy();
		http.onResponse+=onMessage;
	}
	
	public void onMessage(pb_msg mid,byte[] bytes){
		/*
		Loom.QueueOnMainThread(delegate{
			//dispatch to main thread
			Main.Instance.StopWait();
		});
		*/

		switch(mid){
		//Login
		case pb_msg.MsgPcLogin:
			MsgPCLogin msgLogin=MsgPCLogin.Parser.ParseFrom(bytes);
			if(msgLogin.Result==pb_enum.Succeess){
				Debug.Log("response mid="+mid+",uid="+msgLogin.Player.Uid);
				playData.Player=msgLogin.Player;
			}else
				Debug.LogError("login error: "+msgLogin.Result);
			break;
			
		case pb_msg.MsgPcOrder:
			MsgPCOrder msgOrder=MsgPCOrder.Parser.ParseFrom(bytes);
			if(msgOrder.Result==pb_enum.Succeess){
				Debug.Log("response mid="+mid+",scheme="+msgOrder.AppScheme+",ostr="+msgOrder.OrderString);
				//doPay
			}else
				Debug.LogError("order error: "+msgOrder.Result);
			break;
			
		case pb_msg.MsgPcVerify:
			MsgPCVerify msgVerify=MsgPCVerify.Parser.ParseFrom(bytes);
			if(msgVerify.Result==pb_enum.Succeess){
				Debug.Log("response mid="+mid+",uid="+msgVerify.Player.Uid);
				playData.Player=msgVerify.Player;
			}else
				Debug.LogError("verify error: "+msgVerify.Result);
			break;

		default:
			break;
		}
	}

	public void DoVerify(string app_id,string seller_id,string out_trade_no,string total_amount){
		MsgCPVerify msg=new MsgCPVerify();
		msg.Mid=pb_msg.MsgCpVerify;
		msg.AppId=app_id;
		msg.SellerId=seller_id;
		msg.OutTradeNo=out_trade_no;
		msg.TotalAmount=total_amount;
		msg.Uid=playData.Player.Uid;

		//Debug.Log("----DoLogin account="+msg.User.Account);
		Main.Instance.MainPlayer.http.Request<MsgCPVerify>(msg.Mid,msg);

	}
}
