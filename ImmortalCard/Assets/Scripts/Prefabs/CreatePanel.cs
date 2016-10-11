using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	public static CreatePanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	[HideInInspector]
	public GameIcon	Icon;

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

	public void OnClose(){
		Destroy(gameObject);
	}

	public void Connect(uint id=0){
		gameId=id;
		var M=(uint)pb_enum.DefMaxNodes;
		if(id==0){
			//re-generate key
			Random.seed=(int)Utils.time;
			gameId=(uint)(Random.value*M);
		}
		if(Main.Instance.MainPlayer.Connected)
			OnConnected();
		else{
			//connect by key
			var key=gameId%M;
			Main.Instance.MainPlayer.Connect(Configs.ws+"/"+key);
			Debug.Log("connecting by key "+key);
		}
	}

	public void OnConnected(){
		MsgCNEnter msg=new MsgCNEnter();
		msg.Mid=pb_msg.MsgCnEnter;
		msg.Version=100;

		Main.Instance.MainPlayer.Send<MsgCNEnter>(msg.Mid,msg);
	}

	public void OnEntered(){
		if(create){
			var opRobot=new key_value();
			opRobot.Ikey=pb_enum.OptionRobot;
			opRobot.Ivalue=4;
			var opRound=new key_value();
			opRound.Ikey=pb_enum.OptionRound;
			opRound.Ivalue=Main.Instance.Round;

			MsgCNCreate msgC=new MsgCNCreate();
			msgC.Mid=pb_msg.MsgCnCreate;
			msgC.Game=Icon.GameId;
			msgC.Option.Add(opRobot);
			msgC.Option.Add(opRound);

			Main.Instance.MainPlayer.Send<MsgCNCreate>(msgC.Mid,msgC);
			Debug.Log("create game by key "+gameId%(uint)pb_enum.DefMaxNodes);
		}else{
			MsgCNJoin msgJ=new MsgCNJoin();
			msgJ.Mid=pb_msg.MsgCnJoin;
			msgJ.GameId=gameId;

			Main.Instance.MainPlayer.Send<MsgCNJoin>(msgJ.Mid,msgJ);
			Debug.Log("join game by id "+gameId);
		}
	}
	
	public void OnCreated(MsgNCCreate msgC){
		createGame();
	}

	public void OnJoined(MsgNCJoin msgJ){
		if(Icon==null){
			Icon=new GameIcon();
			Icon.GameId=msgJ.Game;
		}
		createGame();
	}

	void createGame(){
		System.Action<Component> handler=delegate(Component obj){
			Destroy(gameObject);
			if(LobbyPanel.Instance!=null)
				Destroy(LobbyPanel.Instance.gameObject);
		};

		if(Icon!=null)
		switch(Icon.GameId){
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
