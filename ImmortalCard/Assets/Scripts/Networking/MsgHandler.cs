using UnityEngine;
using System.Collections;
using Proto3;

public class MsgHandler{
	public delegate void MessageHandler(pb_msg mid,byte[] bytes);

	public bool Connected=false;

	public void onOpen(string error){
		if(!Connected){
			Connected=true;
			Loom.QueueOnMainThread(delegate{
				if(CreatePanel.Instance!=null)
					CreatePanel.Instance.OnConnected();
			});
		}
	}
	public void onClose(string error){
		Connected=false;
		Debug.Log("OnClose "+error);
	}
	public void onError(string error){
		Debug.Log("OnError: "+error);
	}
	public void onMessage(pb_msg mid,byte[] bytes){
		//Debug.Log("OnMessage "+mid);
		switch(mid){
		case pb_msg.MsgScLogin:
			MsgSCLogin msgLogin=MsgSCLogin.Parser.ParseFrom(bytes);
			Debug.Log("response mid="+mid+",uid="+msgLogin.Uid+",ip="+msgLogin.Ip+",port="+msgLogin.Port);
			if(msgLogin.Result==pb_enum.Succeess){
				if(LoginPanel.Instance!=null)LoginPanel.Instance.DoLogin();
			}else
				Debug.LogError("login error: "+msgLogin.Result);
			break;
		case pb_msg.MsgNcEnter:
			MsgNCEnter msgEnter=MsgNCEnter.Parser.ParseFrom(bytes);
			Debug.Log("entered");
			if(msgEnter.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnEntered();
				});
			}else
				Debug.LogError("enter error: "+msgEnter.Result);
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate msgCreate=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+msgCreate.GameId);
			if(msgCreate.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnCreated(msgCreate);
				});
			}else
				Debug.LogError("create error: "+msgCreate.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin msgJoin=MsgNCJoin.Parser.ParseFrom(bytes);
			Debug.Log("joined game");
			if(msgJoin.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnJoined(msgJoin);
				});
			}else
				Debug.LogError("join error: "+msgJoin.Result);
			break;
		case pb_msg.MsgNcStart:
			MsgNCStart msgStart=MsgNCStart.Parser.ParseFrom(bytes);
			Debug.Log("start game");
			if(msgStart.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)
						Main.Instance.StartCoroutine(Main.Instance.gameController.OnMsgStart(msgStart));
				});
			}else
				Debug.LogError("start error: "+msgStart.Result);
			break;

		case pb_msg.MsgNcDiscard:
			MsgNCDiscard msgDiscard=MsgNCDiscard.Parser.ParseFrom(bytes);
			if(msgDiscard.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)
						Main.Instance.StartCoroutine(Main.Instance.gameController.OnMsgDiscard(msgDiscard));
				});
			}else
				Debug.LogError("discard error: "+msgDiscard.Result);
			break;
		case pb_msg.MsgNcMeld:
			MsgNCMeld msgMeld=MsgNCMeld.Parser.ParseFrom(bytes);
			if(msgMeld.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)Main.Instance.gameController.OnMsgMeld(msgMeld);
				});
			}else
				Debug.LogError("meld error: "+msgMeld.Result);
			break;
		case pb_msg.MsgNcDraw:
			MsgNCDraw msgDraw=MsgNCDraw.Parser.ParseFrom(bytes);
			Loom.QueueOnMainThread(delegate{
				if(Main.Instance.gameController!=null)Main.Instance.gameController.OnMsgDraw(msgDraw);
			});
			break;
		case pb_msg.MsgNcSettle:
			MsgNCSettle msgSettle=MsgNCSettle.Parser.ParseFrom(bytes);
			if(msgSettle.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)Main.Instance.gameController.OnMsgSettle(msgSettle);
				});
			}else
				Debug.LogError("settle error: "+msgSettle.Result);
			break;
		case pb_msg.MsgNcFinish:
			MsgNCFinish msgFinish=MsgNCFinish.Parser.ParseFrom(bytes);
			if(msgFinish.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(Main.Instance.gameController!=null)Main.Instance.gameController.OnMsgFinish(msgFinish);
				});
			}else
				Debug.LogError("finish error: "+msgFinish.Result);
			break;
		case pb_msg.MsgNcDismissSync:
			MsgNCDismissSync msgDismissSync=MsgNCDismissSync.Parser.ParseFrom(bytes);
			if(msgDismissSync.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss sync error: "+msgDismissSync.Result);
			break;
		case pb_msg.MsgNcDismissAck:
			MsgNCDismissAck msgDismissAck=MsgNCDismissAck.Parser.ParseFrom(bytes);
			if(msgDismissAck.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss ack error: "+msgDismissAck.Result);
			break;
		default:
			break;
		}
	}
}
