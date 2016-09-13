using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	public static CreatePanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	
	uint Key=0;

	public void OnCreate(){
		Connect();
	}

	public void OnJoin(){
		Utils.Load<GameKeyPopup>(gameObject.transform.parent);
	}

	public void Connect(uint key=0){
		Key=key;
		if(MsgHandler.Connected)
			OnEntered();
		else
			Main.Instance.ws.Connect("ws://127.0.0.1:8820/100");
	}

	public void OnEntered(){
		if(Key>0){
			MsgCNJoin msgJ=new MsgCNJoin();
			msgJ.Mid=pb_msg.MsgCnJoin;
			msgJ.GameId=100;
			msgJ.Key=Key;
			
			Main.Instance.ws.Send<MsgCNJoin>(msgJ.Mid,msgJ);
		}else{
			MsgCNCreate msgC=new MsgCNCreate();
			msgC.Mid=pb_msg.MsgCnCreate;
			msgC.Rule=pb_enum.RuleDdz;
			msgC.Key=Key;
			
			Main.Instance.ws.Send<MsgCNCreate>(msgC.Mid,msgC);
		}
	}
	
	public void OnCreated(MsgNCCreate msgC){
		GamePanel.Create(delegate(Component obj){
			Destroy(gameObject);
		});
	}

	public void OnJoined(MsgNCJoin msgJ){
		GamePanel.Create(delegate(Component obj){
			Destroy(gameObject);
		});
	}
}
