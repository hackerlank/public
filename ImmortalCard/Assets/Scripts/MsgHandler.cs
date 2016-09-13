using UnityEngine;
using System.Collections;
using Proto3;

public class MsgHandler{
	public delegate void MessageHandler(pb_msg mid,byte[] bytes);

	public static bool Connected=false;

	public static void onOpen(string error){
		if(!Connected){
			Connected=true;
			Debug.Log("----OnOpen");
			Loom.QueueOnMainThread(delegate{
				if(CreatePanel.Instance!=null)
					CreatePanel.Instance.OnConnected();
			});
		}
	}
	public static void onClose(string error){
		Connected=false;
		Debug.Log("----OnClose "+error);
	}
	public static void onError(string error){
		Debug.Log("----OnError: "+error);
	}
	public static void onMessage(pb_msg mid,byte[] bytes){
		Debug.Log("----OnMessage "+mid);
		switch(mid){
		case pb_msg.MsgScLogin:
			MsgSCLogin imsg0=MsgSCLogin.Parser.ParseFrom(bytes);
			Debug.Log("response mid="+mid+",uid="+imsg0.Uid+",ip="+imsg0.Ip+",port="+imsg0.Port);
			if(imsg0.Result==pb_enum.Succeess){
				if(LoginPanel.Instance!=null)
					LoginPanel.Instance.DoLogin();
			}else
				Debug.LogError("login error: "+imsg0.Result);
			break;
		case pb_msg.MsgNcEnter:
			MsgNCEnter imsg1=MsgNCEnter.Parser.ParseFrom(bytes);
			Debug.Log("entered game "+imsg1.GameInfo.Gid+",uid="+imsg1.GameInfo.Uid);
			if(imsg1.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)
						CreatePanel.Instance.OnEntered();
				});
			}else
				Debug.LogError("enter error: "+imsg1.Result);
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate imsg2=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+imsg2.GameId);
			if(imsg2.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)
						CreatePanel.Instance.OnCreated(imsg2);
				});
			}else
				Debug.LogError("create error: "+imsg2.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin imsg3=MsgNCJoin.Parser.ParseFrom(bytes);
			Debug.Log("joined game");
			if(imsg3.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)
						CreatePanel.Instance.OnJoined(imsg3);
				});
			}else
				Debug.LogError("join error: "+imsg3.Result);
			break;
		default:
			break;
		}
	}
}
