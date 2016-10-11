using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	public static CreatePanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	[HideInInspector]
	public GameIcon	Icon;

	uint nRobots=0;
	
	public void OnCreate(){
		StartCoroutine(createCo());
	}

	public void OnJoin(){
		StartCoroutine(joinCo());
	}

	public void OnClose(){
		Destroy(gameObject);
	}
	
	IEnumerator createCo(){
		Main.Instance.MainPlayer.Connect();
		while(!Main.Instance.MainPlayer.Entered)yield return null;

		nRobots=4;
		var opRobot=new key_value();
		opRobot.Ikey=pb_enum.OptionRobot;
		opRobot.Ivalue=0;//nRobots;
		var opRound=new key_value();
		opRound.Ikey=pb_enum.OptionRound;
		opRound.Ivalue=Main.Instance.Round;
		
		MsgCNCreate msgC=new MsgCNCreate();
		msgC.Mid=pb_msg.MsgCnCreate;
		msgC.Game=Icon.GameId;
		msgC.Option.Add(opRobot);
		msgC.Option.Add(opRound);
		
		Main.Instance.MainPlayer.Send<MsgCNCreate>(msgC.Mid,msgC);
		Debug.Log("create game by key "+Main.Instance.MainPlayer.gameId%(uint)pb_enum.DefMaxNodes);
		
		while(Main.Instance.MainPlayer.msgNCCreate==null)yield return null;
		createGame();
	}

	IEnumerator joinCo(){
		Utils.Load<GameKeyPopup>(gameObject.transform.parent);
		yield return StartCoroutine(Main.Instance.MainPlayer.JoinGame(Main.Instance.MainPlayer.gameId));
		if(Icon==null){
			Icon=new GameIcon();
			Icon.GameId=Main.Instance.MainPlayer.msgNCJoin.Game;
		}
		createGame();
	}

	void createGame(){
		if(Icon==null)return;
		System.Action<Component> handler=delegate(Component obj){
			Destroy(gameObject);
			if(LobbyPanel.Instance!=null)
				Destroy(LobbyPanel.Instance.gameObject);
		};

		PlayerController ai=null;
		switch(Icon.GameId){
		case pb_enum.GameMj:
			MahJongPanel.Create(handler);
			ai=new MahjongAIController();
			break;
		case pb_enum.GameDdz:
		default:
			DoudeZhuPanel.Create(handler);
			ai=new DoudeZhuAIController();
			break;
		}
		Main.Instance.MainPlayer.controllers.Add(Main.Instance.gameController);

		if(nRobots>0){
			//add robots demand
			var MP=Main.Instance.gameController.Rule.MaxPlayer;
			if(nRobots>=MP)nRobots=MP-1;
			for(uint i=0;i<nRobots;++i){
				var robot=new Player(true);
				robot.controllers.Add(ai);
				Main.Instance.players.Add(robot);
				Main.Instance.StartCoroutine(robot.JoinGame(Main.Instance.MainPlayer.msgNCCreate.GameId));
			}
		}
	}
}
