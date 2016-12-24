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
	public Transform	OptionRoot;
	public Text			Information;

	[HideInInspector]
	public pb_enum	CurrentGame;

	RuleIcon icon;
	public RuleIcon	GameCategory{
		get{
			return icon;
		}
		set{
			icon=value;
			StartCoroutine(loadOption());
		}
	}

	public static EnterPanel Instance=null;
	void Awake(){
		DefinedCards.text=PlayerPrefs.GetString(Cache.PrefsKey_DefinedCards);
		Instance=this;
	}
	void OnDestroy(){Instance=null;}

	IEnumerator Start(){
		if(Config.games==null || !Config.games.ContainsKey(CurrentGame))
			yield break;

		if(null==Config.options)
			StartCoroutine(Main.Instance.updater.Load<TextAsset>(
				"Config/options",null,delegate(Object obj,Hashtable param){
				var ta=obj as TextAsset;
				Config.LoadOptions(ta.text);
			}));

		while(null==RuleSprites.Instance || null==Config.options)
			yield return null;

		//game icons
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

		//options
		yield return StartCoroutine(loadOption());
	}

	IEnumerator loadOption(){
		foreach(Transform ch in OptionRoot)
			Destroy(ch.gameObject);
		yield return null;

		var categoryOptions=Config.options[CurrentGame];
		if(categoryOptions!=null){
			var optionGroups=categoryOptions[GameCategory.Value];
			if(optionGroups!=null){
				foreach(var _optionGroup in optionGroups){
					//option group
					MonoBehaviour group=null;
					StartCoroutine(Main.Instance.updater.Load<MonoBehaviour>(
						"Prefabs/OptionGroup",OptionRoot,delegate(Object obj,Hashtable arg){
						group=obj as MonoBehaviour;
					}));
					
					while(group==null)yield return null;
					
					var optionGroup=_optionGroup;
					
					foreach(var option in optionGroup){
						var radios=option["option"] as ArrayList;
						if(radios !=null){
							//group
							ToggleGroup tg=null;
							foreach(var radio in radios){
								var radioHash=radio as Hashtable;
								if(radioHash!=null && radioHash.Count>0){
									Option Opt=null;
									StartCoroutine(Main.Instance.updater.Load<Option>(
										"Prefabs/Option",group.transform,delegate(Object obj1,Hashtable arg1){
										Opt=(obj1 as Option);
									}));
									
									while(Opt==null)
										yield return null;
									
									Opt.checker.sprite=Opt.radio.sprite;
									Opt.checkerMarker.sprite=Opt.radioMarker.sprite;
									if(null==tg){
										tg=Opt.toggle.group;
										Opt.toggle.isOn=true;
									}else{
										Opt.toggle.group=tg;
										Opt.toggle.isOn=false;
									}
									Opt.Value=radioHash;
								}
							}
						}else{
							//option
							var para=new Hashtable();
							para["option"]=option;
							StartCoroutine(Main.Instance.updater.Load<Option>(
								"Prefabs/Option",group.transform,delegate(Object obj1,Hashtable arg1){
								var Opt=(obj1 as Option);
								Destroy(Opt.toggle.group);
								Opt.Value=arg1["option"] as Hashtable;
							},para));
						}//option
					}
				}
			}
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
		StartCoroutine(Main.Instance.updater.Load<ReplayView>(
			"Prefabs/ReplayView",Main.Instance.RootPanel,delegate(Object arg1, Hashtable arg2) {
			var msg=new MsgCLReplays();
			msg.Mid=pb_msg.MsgClReplays;
			msg.Session=Main.Instance.MainPlayer.session;
			msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
			Main.Instance.MainPlayer.http.Request<MsgCLReplays>(msg.Mid,msg);
		}));
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

		//game options
		var opRound=new key_value();
		opRound.Key="round";
		opRound.Ivalue=Main.Instance.Round;

		Main.Instance.MainPlayer.category=GameCategory.Value;
		var opCategory=new key_value();
		opCategory.Key="category";
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
			opCards.Key="defined_cards";
			opCards.Value=cards;
			msgC.Options.Add(opCards);

			//save
			PlayerPrefs.SetString(Cache.PrefsKey_DefinedCards,cards);
			PlayerPrefs.Save();
		}

		//user options
		var options=OptionRoot.GetComponentsInChildren<Option>();
		foreach(var Opt in options){
			if(Opt.toggle.isOn)
				msgC.Options.Add(Opt.KeyValue);
		}

		//send message
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
}
