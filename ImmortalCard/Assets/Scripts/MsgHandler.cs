using UnityEngine;
using System.Collections;
using Proto3;

public class MsgHandler{
	public delegate void MessageHandler(pb_msg mid,byte[] bytes);

	public static bool Connected=false;

	public static void onOpen(string error){
		if(!Connected){
			Connected=true;
			Loom.QueueOnMainThread(delegate{
				if(CreatePanel.Instance!=null)
					CreatePanel.Instance.OnConnected();
			});
		}
	}
	public static void onClose(string error){
		Connected=false;
		Debug.Log("OnClose "+error);
	}
	public static void onError(string error){
		Debug.Log("OnError: "+error);
	}
	public static void onMessage(pb_msg mid,byte[] bytes){
		Debug.Log("OnMessage "+mid);
		switch(mid){
		case pb_msg.MsgScLogin:
			MsgSCLogin imsg0=MsgSCLogin.Parser.ParseFrom(bytes);
			Debug.Log("response mid="+mid+",uid="+imsg0.Uid+",ip="+imsg0.Ip+",port="+imsg0.Port);
			if(imsg0.Result==pb_enum.Succeess){
				if(LoginPanel.Instance!=null)LoginPanel.Instance.DoLogin();
			}else
				Debug.LogError("login error: "+imsg0.Result);
			break;
		case pb_msg.MsgNcEnter:
			MsgNCEnter imsg1=MsgNCEnter.Parser.ParseFrom(bytes);
			Debug.Log("entered");
			if(imsg1.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnEntered();
				});
			}else
				Debug.LogError("enter error: "+imsg1.Result);
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate imsg2=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+imsg2.GameId);
			if(imsg2.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnCreated(imsg2);
				});
			}else
				Debug.LogError("create error: "+imsg2.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin imsg3=MsgNCJoin.Parser.ParseFrom(bytes);
			Debug.Log("joined game");
			if(imsg3.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnJoined(imsg3);
				});
			}else
				Debug.LogError("join error: "+imsg3.Result);
			break;
		case pb_msg.MsgNcStart:
			MsgNCStart imsg4=MsgNCStart.Parser.ParseFrom(bytes);
			Debug.Log("start game");
			if(imsg4.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)
						Main.Instance.StartCoroutine(Main.Instance.gameController.Deal(imsg4));
				});
			}else
				Debug.LogError("start error: "+imsg4.Result);
			break;

		case pb_msg.MsgNcDiscard:
			MsgNCDiscard imsg5=MsgNCDiscard.Parser.ParseFrom(bytes);
			if(imsg5.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)
						Main.Instance.StartCoroutine(Main.Instance.gameController.OnDiscardAt(imsg5));
				});
			}else
				Debug.LogError("discard error: "+imsg5.Result);
			break;
		case pb_msg.MsgNcMeld:
			MsgNCMeld imsg6=MsgNCMeld.Parser.ParseFrom(bytes);
			if(imsg6.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("meld error: "+imsg6.Result);
			break;
		case pb_msg.MsgNcSettle:
			MsgNCSettle imsg7=MsgNCSettle.Parser.ParseFrom(bytes);
			if(imsg7.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)Main.Instance.gameController.OnSettle(imsg7);
				});
			}else
				Debug.LogError("settle error: "+imsg7.Result);
			break;
		case pb_msg.MsgNcFinish:
			MsgNCFinish imsg8=MsgNCFinish.Parser.ParseFrom(bytes);
			if(imsg8.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)Main.Instance.gameController.OnFinish(imsg8);
				});
			}else
				Debug.LogError("finish error: "+imsg8.Result);
			break;
		case pb_msg.MsgNcDismissSync:
			MsgNCDismissSync imsg9=MsgNCDismissSync.Parser.ParseFrom(bytes);
			if(imsg9.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss sync error: "+imsg9.Result);
			break;
		case pb_msg.MsgNcDismissAck:
			MsgNCDismissAck imsg10=MsgNCDismissAck.Parser.ParseFrom(bytes);
			if(imsg10.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss ack error: "+imsg10.Result);
			break;
		default:
			break;
		}
	}
}
