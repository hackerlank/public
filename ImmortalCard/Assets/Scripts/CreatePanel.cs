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

		Main.Instance.ws.Send<MsgCNEnter>(msg.Mid,msg);
	}

	public void OnEntered(){
		if(create){
			var opRobot=new key_value();
			opRobot.Ikey=pb_enum.OptionRobot;
			opRobot.Ivalue=2;
			var opRound=new key_value();
			opRound.Ikey=pb_enum.OptionRound;
			opRound.Ivalue=Main.Instance.Round;

			//select game
			Main.Instance.game=pb_enum.GameDdz;

			MsgCNCreate msgC=new MsgCNCreate();
			msgC.Mid=pb_msg.MsgCnCreate;
			msgC.Game=Main.Instance.game;
			msgC.Option.Add(opRobot);
			msgC.Option.Add(opRound);

			Main.Instance.ws.Send<MsgCNCreate>(msgC.Mid,msgC);
			Debug.Log("create game by key "+gameId%(uint)pb_enum.DefMaxNodes);
		}else{
			MsgCNJoin msgJ=new MsgCNJoin();
			msgJ.Mid=pb_msg.MsgCnJoin;
			msgJ.GameId=gameId;

			Main.Instance.ws.Send<MsgCNJoin>(msgJ.Mid,msgJ);
			Debug.Log("join game by id "+gameId);
		}
	}
	
	public void OnCreated(MsgNCCreate msgC){
		createGame();
	}

	public void OnJoined(MsgNCJoin msgJ){
		Main.Instance.game=msgJ.Game;
		createGame();
	}

	void createGame(){
		System.Action<Component> handler=delegate(Component obj){
			Destroy(gameObject);
		};

		switch(Main.Instance.game){
		case pb_enum.GameMj:
			MahJongPanel.Create(handler);
			break;
		case pb_enum.GameDdz:
		default:
			DoudeZhuPanel.Create(handler);
			break;
		}
	}
}
