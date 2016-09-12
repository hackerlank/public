using UnityEngine;
using System.Collections;
using Proto3;

public class MsgHandler{
	public delegate void MessageHandler(uint mid,byte[] bytes);

	public static void onOpen(string error){
		Debug.Log("----OnOpen");
	}
	public static void onClose(string error){
		Debug.Log("----OnClose "+error);
	}
	public static void onError(string error){
		Debug.Log("----OnError: "+error);
	}
	public static void onMessage(uint mid,byte[] bytes){
		Debug.Log("----OnMessage "+mid);
		switch(mid){
		case 2002:
			MsgSCLogin msg=MsgSCLogin.Parser.ParseFrom(bytes);
			Debug.Log("response mid="+mid+",uid="+msg.Uid+",ip="+msg.Ip+",port="+msg.Port);
			break;
		case 6002:
			MsgNCEnter imsg=MsgNCEnter.Parser.ParseFrom(bytes);
			Debug.Log("entered game "+imsg.GameInfo.Gid+",uid="+imsg.GameInfo.Uid);
			break;
		default:
			break;
		}
	}
}
