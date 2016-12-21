using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class EnterPanel : MonoBehaviour {
	public InputField	DefinedCards;
	public Toggle		CreateTap,JoinTap;
	public GameObject	GameContent,GameOptions;
	public GameObject	DialPanel,InputPanel;
	public Transform	GameRoot;
	public Text			Information;

	[HideInInspector]
	public pb_enum	CurrentGame;
	[HideInInspector]
	public RuleIcon	GameCategory;

	public static EnterPanel Instance=null;
	void Awake(){
		DefinedCards.text=PlayerPrefs.GetString(Cache.PrefsKey_DefinedCards);
		Instance=this;
	}
	void OnDestroy(){Instance=null;}

	IEnumerator Start(){
		if(!Config.games.ContainsKey(CurrentGame))
			yield break;

		//if(!string.IsNullOrEmpty(CurrentRule.Desc))
		//	Information.text=CurrentRule.Desc;

		while(null==RuleSprites.Instance)yield return null;

		var categories=Config.games[CurrentGame];
		foreach(game_t game in categories){
			var param=new Hashtable();
			param["rule"]=game.Rule%100;
			StartCoroutine(Main.Instance.updater.Load<RuleIcon>(
				"Prefabs/RuleIcon",GameRoot,delegate(Object obj,Hashtable arg){
				var icon=obj as RuleIcon;
				icon.Value=(pb_enum)arg["rule"];

				//default selection
				if(GameCategory==null){
					GameCategory=icon;
					GameCategory.OnGame();
				}
			},param));
			yield return null;
		}
	}

	public void OnCreate(){
		GameContent.SetActive(true);
		GameOptions.SetActive(true);
		DialPanel.SetActive(false);
		InputPanel.SetActive(false);
	}

	public void OnJoin(){
		GameContent.SetActive(false);
		GameOptions.SetActive(false);
		DialPanel.SetActive(true);
		InputPanel.SetActive(true);
	}

	public void OnOK(){
		if(!BlockView.Instance.Blocking){
			BlockView.Instance.Blocking=true;
			if(CreateTap.isOn)
				StartCoroutine(createCo());
			else
				StartCoroutine(joinCo());
		}
	}
	
	public void OnDial(int n){
	}

	public void OnHelp(){
		StartCoroutine(
			Main.Instance.updater.Load<HelpPanel>("Prefabs/HelpPanel",Main.Instance.RootPanel));
	}

	public void OnReplay(){
		StartCoroutine(replayCo());
	}

	IEnumerator replayCo(){
		//DoReplay(null);
		//yield break;
		//connect: unstable and only for test
		if(!Main.Instance.MainPlayer.InGame)
			Main.Instance.MainPlayer.Connect(100);
		while(!Main.Instance.MainPlayer.InGame)yield return null;

		var msg=new MsgCLReplay();
		msg.Mid=pb_msg.MsgClReplay;
		msg.Session=Main.Instance.MainPlayer.session;
		msg.GameId=10000;
		Main.Instance.MainPlayer.Send<MsgCLReplay>(msg.Mid,msg);
	}
	
	public void OnTest(){
		DefinedCards.gameObject.SetActive(!DefinedCards.gameObject.activeSelf);
	}

	public void OnBack(){
		StartCoroutine(Main.Instance.updater.Load<LobbyPanel>(
			"Prefabs/LobbyPanel",Main.Instance.RootPanel,delegate(Object obj,Hashtable arg){
			Destroy(gameObject);
		}));
	}
	
	IEnumerator createCo(){
		Main.Instance.MainPlayer.Connect();
		while(!Main.Instance.MainPlayer.InGame)yield return null;

		var storeGame=new StoreGame();
		Cache.storeGame=storeGame;
		storeGame.gameType=(int)CurrentGame;
		storeGame.robots=4;

		var opRound=new key_value();
		opRound.Ikey=pb_enum.OptionRound;
		opRound.Ivalue=Main.Instance.Round;

		Main.Instance.MainPlayer.category=GameCategory.Value;
		var opCategory=new key_value();
		opCategory.Ikey=pb_enum.OptionCategory;
		opCategory.Ivalue=(int)Main.Instance.MainPlayer.category;
		
		MsgCNCreate msgC=new MsgCNCreate();
		msgC.Mid=pb_msg.MsgCnCreate;
		msgC.Game=CurrentGame;
		if(msgC.Game>pb_enum.GameMj)
			msgC.Game=pb_enum.GameMj;

		msgC.Options.Add(opRound);
		msgC.Options.Add(opCategory);

		if(DefinedCards.text.Length>0){
			//preprocess
			var cards=DefinedCards.text;
			cards.Replace(" ","");
			cards.Replace("\n","");

			var opCards=new key_value();
			opCards.Ikey=pb_enum.OptionDefinedCards;
			opCards.Value=cards;
			msgC.Options.Add(opCards);

			//save
			PlayerPrefs.SetString(Cache.PrefsKey_DefinedCards,cards);
			PlayerPrefs.Save();
		}
		
		Main.Instance.MainPlayer.msgNCCreate=null;
		Main.Instance.MainPlayer.Send<MsgCNCreate>(msgC.Mid,msgC);
		//Debug.Log("create game by key "+Main.Instance.MainPlayer.gameId%(uint)pb_enum.DefMaxNodes);

		while(Main.Instance.MainPlayer.msgNCCreate==null)yield return null;
		storeGame.gameId=Main.Instance.MainPlayer.msgNCCreate.GameId;

		Main.Instance.MainPlayer.msgNCCreate=null;
		yield return StartCoroutine(createGame());
	}

	IEnumerator joinCo(){
		Main.Instance.MainPlayer.msgNCJoin=null;
		while(Main.Instance.MainPlayer.msgNCJoin==null)yield return null;

		var storeGame=Cache.storeGame;
		storeGame.gameType=(int)Main.Instance.MainPlayer.msgNCJoin.Game;
		Main.Instance.MainPlayer.msgNCJoin=null;

		yield return StartCoroutine(createGame());
	}

	IEnumerator createGame(){
		//create game panel
		var storeGame=Cache.storeGame;
		yield return StartCoroutine(Main.Instance.MainPlayer.CreateGame(
			(pb_enum)storeGame.gameType,storeGame.gameId));

		//add robots and join
		Player.addRobots(storeGame.robots);
		var robots=new List<Player>(Main.Instance.robots);
		foreach(var robot in robots)
			yield return StartCoroutine(robot.JoinGame(storeGame.gameId));

		if(LobbyPanel.Instance!=null)
			Destroy(LobbyPanel.Instance.gameObject);
		Destroy(gameObject);
	}

	public void DoReplay(MsgLCReplay msg){
		ReplayPanel panel=null;
		GameRule rule=null;
		System.Action<Component> handler=delegate(Component obj) {
			Destroy(gameObject);
			panel=obj as ReplayPanel;
			panel.Rule=rule;
			panel.StartCoroutine(panel.Play(msg));
		};

		switch(CurrentGame){
		case pb_enum.GameMjChengdu:
			rule=new MahJongRule();
			ReplayPanel.Create("MahjongPanel",handler);
			break;
		case pb_enum.GameDdz:
			rule=new DoudeZhuRule();
			ReplayPanel.Create("MahjongPanel",handler);
			//PaohuziReplay.Create("DoudeZheReplay");
			break;
		case pb_enum.GamePhz:
		default:
			rule=new PaohuziRule();
			ReplayPanel.Create("PaohuziReplay",handler);
			break;
		}
	}
}
