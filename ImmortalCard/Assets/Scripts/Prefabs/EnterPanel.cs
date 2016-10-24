using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class EnterPanel : MonoBehaviour {
	public InputField DefinedCards;
	public GameObject CreateTab,JoinTab;
	public Transform GameRoot;
	public Text Information;

	public static EnterPanel Instance=null;
	void Awake(){
		DefinedCards.text=PlayerPrefs.GetString(Configs.PrefsKey_DefinedCards);
		Instance=this;
	}
	void OnDestroy(){Instance=null;}
	[HideInInspector]
	public GameIcon	CurrentGame;
	[HideInInspector]
	public RuleIcon	Icon;

	int nRobots=0;

	void Start(){
		game_t game=new game_t();
		game.Id=(int)pb_enum.GameDdz;
		addGame(game);
		game=new game_t();
		game.Id=(int)pb_enum.GameMj;
		addGame(game);
		game=new game_t();
		game.Id=(int)pb_enum.GamePhz;
		addGame(game);
		for(int i=0;i<0;++i){
			game=new game_t();
			game.Id=(int)pb_enum.GameMj;
			addGame(game);
		}
	}
	
	void addGame(game_t game){
		Utils.Load<RuleIcon>(GameRoot,delegate(Component obj){
			var icon=obj as RuleIcon;
			icon.GameId=(pb_enum)game.Id;
			icon.Name.text=icon.GameId.ToString().Substring(4);
			if(Icon==null){
				Icon=icon;
				Icon.OnGame();
			}
		});
	}

	public void OnCreate(){
		CreateTab.SetActive(true);
		JoinTab.SetActive(false);
	}

	public void OnJoin(){
		CreateTab.SetActive(false);
		JoinTab.SetActive(true);
	}

	public void OnCreateOK(){
		StartCoroutine(createCo());
	}
	
	public void OnJoinOK(){
		StartCoroutine(joinCo());
	}
	
	public void OnDial(int n){
	}

	public void OnHelp(){
	}

	public void OnLog(){
	}

	public void OnBack(){
		Utils.Load<LobbyPanel>(gameObject.transform.parent,delegate(Component obj){
			Destroy(gameObject);
		});
	}
	
	IEnumerator createCo(){
		Main.Instance.MainPlayer.Connect();
		while(!Main.Instance.MainPlayer.Entered)yield return null;

		nRobots=4;
		var opRound=new key_value();
		opRound.Ikey=pb_enum.OptionRound;
		opRound.Ivalue=Main.Round;

		Main.Instance.MainPlayer.category=pb_enum.PhzSy;
		var opCategory=new key_value();
		opCategory.Ikey=pb_enum.OptionCategory;
		opCategory.Ivalue=(int)Main.Instance.MainPlayer.category;
		
		MsgCNCreate msgC=new MsgCNCreate();
		msgC.Mid=pb_msg.MsgCnCreate;
		msgC.Game=Icon.GameId;
		msgC.Options.Add(opRound);
		msgC.Options.Add(opCategory);

		if(DefinedCards.text.Length>0){
			var opCards=new key_value();
			opCards.Ikey=pb_enum.OptionDefinedCards;
			opCards.Value=DefinedCards.text;
			msgC.Options.Add(opCards);

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
		while(Main.Instance.MainPlayer.msgNCJoin==null)yield return null;
		if(Icon==null){
			Icon=new RuleIcon();
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
		case pb_enum.GamePhz:
			PaohuziPanel.Create(handler);
			break;
		case pb_enum.GameDdz:
		default:
			DoudeZhuPanel.Create(handler);
			break;
		}
	}
}
