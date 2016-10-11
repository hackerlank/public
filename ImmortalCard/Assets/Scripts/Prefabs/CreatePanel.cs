using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	public static CreatePanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	[HideInInspector]
	public GameIcon	Icon;

	uint gameId=0;

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
		while(!Main.Instance.MainPlayer.Connected)yield return null;

		while(!Main.Instance.MainPlayer.Entered)yield return null;
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
		
		while(Main.Instance.MainPlayer.msgNCCreate==null)yield return null;
		createGame();
	}

	IEnumerator joinCo(){
		Utils.Load<GameKeyPopup>(gameObject.transform.parent);
		while(!Main.Instance.MainPlayer.Connected)yield return null;

		while(!Main.Instance.MainPlayer.Entered)yield return null;
		MsgCNJoin msgJ=new MsgCNJoin();
		msgJ.Mid=pb_msg.MsgCnJoin;
		msgJ.GameId=gameId;
		
		Main.Instance.MainPlayer.Send<MsgCNJoin>(msgJ.Mid,msgJ);
		Debug.Log("join game by id "+gameId);

		while(Main.Instance.MainPlayer.msgNCJoin==null)yield return null;
		if(Icon==null){
			Icon=new GameIcon();
			Icon.GameId=Main.Instance.MainPlayer.msgNCJoin.Game;
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
