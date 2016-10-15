using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {
	public InputField DefinedCards;

	public static CreatePanel Instance=null;
	void Awake(){
		DefinedCards.text=PlayerPrefs.GetString(Configs.PrefsKey_DefinedCards);
		Instance=this;
	}
	void OnDestroy(){Instance=null;}
	[HideInInspector]
	public GameIcon	Icon;

	int nRobots=0;
	
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
		var opRound=new key_value();
		opRound.Ikey=pb_enum.OptionRound;
		opRound.Ivalue=Main.Round;
		
		MsgCNCreate msgC=new MsgCNCreate();
		msgC.Mid=pb_msg.MsgCnCreate;
		msgC.Game=Icon.GameId;
		msgC.Option.Add(opRound);

		if(DefinedCards.text.Length>0){
			var opCards=new key_value();
			opCards.Ikey=pb_enum.OptionDefinedCards;
			opCards.Value=DefinedCards.text;
			msgC.Option.Add(opCards);

			//save
			PlayerPrefs.SetString(Configs.PrefsKey_DefinedCards,DefinedCards.text);
			PlayerPrefs.Save();
		}
		
		Main.Instance.MainPlayer.msgNCCreate=null;
		Main.Instance.MainPlayer.Send<MsgCNCreate>(msgC.Mid,msgC);
		//Debug.Log("create game by key "+Main.Instance.MainPlayer.gameId%(uint)pb_enum.DefMaxNodes);

		while(Main.Instance.MainPlayer.msgNCCreate==null)yield return null;
		var gameId=Main.Instance.MainPlayer.msgNCCreate.GameId;
		Main.Instance.MainPlayer.msgNCCreate=null;
		createGame(gameId);
	}

	IEnumerator joinCo(){
		Main.Instance.MainPlayer.msgNCJoin=null;
		Utils.Load<GameKeyPopup>(gameObject.transform.parent);

		while(Main.Instance.MainPlayer.msgNCJoin==null)yield return null;
		if(Icon==null){
			Icon=new GameIcon();
			Icon.GameId=Main.Instance.MainPlayer.msgNCJoin.Game;
		}
		Main.Instance.MainPlayer.msgNCJoin=null;
		createGame(Main.Instance.MainPlayer.gameId);
	}

	void createGame(int gameId){
		if(Icon==null)return;

		System.Action<Component> handler=delegate(Component obj){
			Destroy(gameObject);
			if(LobbyPanel.Instance!=null)
				Destroy(LobbyPanel.Instance.gameObject);

			if(nRobots>0){
				//add robots demand
				var panel=obj as GamePanel;

				var MP=panel.Rule.MaxPlayer;
				if(nRobots>=MP)nRobots=MP-1;
				for(uint i=0;i<nRobots;++i){
					var robot=new Player();
					robot.controllers.Add(panel.Rule.AIController);
					Main.Instance.players.Add(robot);
					panel.StartCoroutine(robot.JoinGame(gameId));
				}
			}
		};

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
