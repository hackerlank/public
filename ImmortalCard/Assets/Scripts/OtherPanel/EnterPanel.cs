using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
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
	public RuleIcon	GameCategory;

	void Start(){
		var phz=new Dictionary<pb_enum, string>();
		phz[pb_enum.PhzSy]="邵阳字牌";
		phz[pb_enum.PhzSybp]="邵阳剥皮";
		phz[pb_enum.PhzLd]="娄底放炮";
		phz[pb_enum.PhzHh]="怀化红拐弯";
		phz[pb_enum.PhzCdQmt]="常德全名堂";
		phz[pb_enum.PhzCdHhd]="常德红黑点";
		phz[pb_enum.PhzCs]="长沙";
		phz[pb_enum.PhzXxGhz]="湘乡告胡子";
		phz[pb_enum.PhzHy]="衡阳六条枪";
		phz[pb_enum.PhzYzSbw]="永州双霸王";
		phz[pb_enum.PhzPeghz]="碰胡子";
		phz[pb_enum.PhzScEqs]="四川二七十";
		phz[pb_enum.PhzCz]="郴州跑胡子";
		phz[pb_enum.PhzGx]="广西跑胡子";

		var mj=new Dictionary<pb_enum, string>();
		mj[pb_enum.MjSichuan]="四川麻将";
		mj[pb_enum.MjGuangdong]="广东麻将";
		mj[pb_enum.MjZhejiang]="浙江麻将";

		var ddz=new Dictionary<pb_enum, string>();
		ddz[pb_enum.PhzSy]="经典斗地主";
		ddz[pb_enum.PhzSybp]="四人斗地主";

		var games=phz;
		switch(CurrentGame.game){
		case pb_enum.GameMj:
			games=mj;
			break;
		case pb_enum.GameDdz:
			games=ddz;
			break;
		case pb_enum.GamePhz:
		default:
			games=phz;
			break;
		}
		foreach(var kv in games)
			Utils.Load<RuleIcon>(GameRoot,delegate(Component obj){
				var category=kv.Key;
				var icon=obj as RuleIcon;
				icon.Category=category;
				icon.Name.text=games[category];
				if(GameCategory==null){
					GameCategory=icon;
					GameCategory.OnGame();
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
		if(!Main.Instance.Wait){
			Main.Instance.Wait=true;
			StartCoroutine(createCo());
		}
	}
	
	public void OnJoinOK(){
		if(!Main.Instance.Wait){
			Main.Instance.Wait=true;
			StartCoroutine(joinCo());
		}
	}
	
	public void OnDial(int n){
	}

	public void OnHelp(){
	}

	public void OnLog(){
		var msg=new MsgCLReplay();
		msg.Mid=pb_msg.MsgLcReplay;
		msg.GameId=10000;
		Main.Instance.MainPlayer.Send<MsgCLReplay>(msg.Mid,msg);
	}
	
	public void OnTest(){
		DefinedCards.gameObject.SetActive(!DefinedCards.gameObject.activeSelf);
	}

	public void OnBack(){
		Utils.Load<LobbyPanel>(Main.Instance.RootPanel,delegate(Component obj){
			Destroy(gameObject);
		});
	}
	
	IEnumerator createCo(){
		Main.Instance.MainPlayer.Connect();
		while(!Main.Instance.MainPlayer.InGame)yield return null;

		var storeGame=new StoreGame();
		Main.Instance.storeGame=storeGame;
		storeGame.gameType=(int)CurrentGame.game;
		storeGame.robots=4;

		var opRound=new key_value();
		opRound.Ikey=pb_enum.OptionRound;
		opRound.Ivalue=Main.Round;

		Main.Instance.MainPlayer.category=GameCategory.Category;
		var opCategory=new key_value();
		opCategory.Ikey=pb_enum.OptionCategory;
		opCategory.Ivalue=(int)Main.Instance.MainPlayer.category;
		
		MsgCNCreate msgC=new MsgCNCreate();
		msgC.Mid=pb_msg.MsgCnCreate;
		msgC.Game=CurrentGame.game;

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
			PlayerPrefs.SetString(Configs.PrefsKey_DefinedCards,cards);
			PlayerPrefs.Save();
		}
		
		Main.Instance.MainPlayer.msgNCCreate=null;
		Main.Instance.MainPlayer.Send<MsgCNCreate>(msgC.Mid,msgC);
		//Debug.Log("create game by key "+Main.Instance.MainPlayer.gameId%(uint)pb_enum.DefMaxNodes);

		while(Main.Instance.MainPlayer.msgNCCreate==null)yield return null;
		Debug.Log("----store game id="+storeGame.gameId);
		storeGame.gameId=Main.Instance.MainPlayer.msgNCCreate.GameId;

		Main.Instance.MainPlayer.msgNCCreate=null;
		yield return StartCoroutine(createGame());
	}

	IEnumerator joinCo(){
		Main.Instance.MainPlayer.msgNCJoin=null;
		while(Main.Instance.MainPlayer.msgNCJoin==null)yield return null;

		var storeGame=Main.Instance.storeGame;
		storeGame.gameType=(int)Main.Instance.MainPlayer.msgNCJoin.Game;
		Main.Instance.MainPlayer.msgNCJoin=null;

		yield return StartCoroutine(createGame());
	}

	IEnumerator createGame(){
		//create game panel
		var storeGame=Main.Instance.storeGame;
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

	public IEnumerator DoReplay(MsgLCReplay msg){
		ReplayPanel panel=null;
		System.Action<Component> handler=delegate(Component obj) {
			panel=obj as ReplayPanel;
			panel.StartCoroutine(panel.Play(msg));
		};

		switch(CurrentGame.game){
		case pb_enum.GameMj:
			ReplayPanel.Create("MahjongPanel",handler);
			break;
		case pb_enum.GameDdz:
			ReplayPanel.Create("MahjongPanel",handler);
			//ReplayPanel.Create("DoudeZheReplay");
			break;
		case pb_enum.GamePhz:
		default:
			ReplayPanel.Create("PaohuziReplay",handler);
			break;
		}
		while(null==panel)yield return null;
		Destroy(gameObject);
	}
}
