using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	public static CreatePanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	bool create=true;
	uint gameId=0;

	public void OnCreate(){
		create=true;
		Connect();
	}

	public void OnJoin(){
		create=false;
		Utils.Load<GameKeyPopup>(gameObject.transform.parent);
	}

	public void Connect(uint id=0){
		gameId=id;
		var M=(uint)pb_enum.DefMaxNodes;
		if(id==0){
			//re-generate key
			Random.seed=(int)Utils.time;
			gameId=(uint)(Random.value*M);
		}
		if(MsgHandler.Connected)
			OnConnected();
		else{
			//connect by key
			var key=gameId%M;
			Main.Instance.ws.Connect(Configs.ws+"/"+key);
			Debug.Log("connecting by key "+key);
		}
	}

	public void OnConnected(){
		MsgCNEnter msg=new MsgCNEnter();
		msg.Mid=pb_msg.MsgCnEnter;
		msg.Version=100;
		msg.Key=gameId%(uint)pb_enum.DefMaxNodes;
		
		Main.Instance.ws.Send<MsgCNEnter>(msg.Mid,msg);
	}

	public void OnEntered(){
		if(create){
			MsgCNCreate msgC=new MsgCNCreate();
			msgC.Mid=pb_msg.MsgCnCreate;
			msgC.Rule=pb_enum.RuleDdz;
			msgC.Key=gameId%(uint)pb_enum.DefMaxNodes;
			
			Main.Instance.ws.Send<MsgCNCreate>(msgC.Mid,msgC);
			Debug.Log("create game by key "+msgC.Key);
		}else{
			MsgCNJoin msgJ=new MsgCNJoin();
			msgJ.Mid=pb_msg.MsgCnJoin;
			msgJ.GameId=gameId;
			msgJ.Key=gameId%(uint)pb_enum.DefMaxNodes;
			
			Main.Instance.ws.Send<MsgCNJoin>(msgJ.Mid,msgJ);
			Debug.Log("join game by id "+gameId);
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
