using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class LobbyPanel : MonoBehaviour {

	public Text			Bulletin;
	public Transform	GameRoot;
	public ContentSizeFitter	Content;

	public static LobbyPanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	IEnumerator Start(){
		var str=PlayerPrefs.GetString(Cache.PrefsKey_StoreGame,"");
		var storeGame=new StoreGame();
		storeGame.FromString(str);

		if(storeGame.gameId>0){
			Debug.Log("found game "+storeGame.gameId);
			//in game: reconnect and create game panel,robots
			Cache.storeGame=storeGame;
			yield return StartCoroutine(Main.Instance.MainPlayer.Reconnect());

			Destroy(gameObject);
		}else{
			yield return StartCoroutine(loadLobbyCo());
		}
	}

	IEnumerator loadLobbyCo(){
		var info=Info;
		if(Dirty){
			MsgCLEnter msg=new MsgCLEnter();
			msg.Mid=pb_msg.MsgClEnter;
			msg.Version=uint.Parse(Config.build);
			msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
			Main.Instance.MainPlayer.http.Request<MsgCLEnter>(msg.Mid,msg);

			Info=null;
		}

		while(Info==null)
			yield return null;

		if(info!=null && Info.Version<=info.Version)
			Info=info;

		//to enter panel
		var games=new pb_enum[]{pb_enum.GameDdz,pb_enum.GameMj,pb_enum.GamePhz};
		foreach(var id in games){
			game_t game=new game_t();
			game.Id=(int)id;
			addGame(game);
		}
	}

	void addGame(game_t game){
		StartCoroutine(Main.Instance.updater.Load<GameIcon>(
			"Prefabs/GameIcon",GameRoot,delegate(Object obj,Hashtable arg){
			var icon=obj as GameIcon;
			icon.game=(pb_enum)game.Id;
			switch(icon.game){
			case pb_enum.GameMj:
				icon.Name.text="Mahjong";	break;
			case pb_enum.GameDdz:
				icon.Name.text="DoudeZhu";	break;
			case pb_enum.GamePhz:
			default:
				icon.Name.text="Paohuzi";	break;
			}
		}));
	}

	public void OnIcon(){
	}
	
	public void OnCurrency(){
	}
	
	public void OnMail(){
		Main.Instance.share.Share("Share","I'm fun!",cn.sharesdk.unity3d.ContentType.Image);
	}
	
	public void OnSettings(){
		StartCoroutine(Main.Instance.updater.Load<SettingsPanel>(
			"Prefabs/SettingsPanel",Main.Instance.RootPanel));
	}
	
	public void OnProxy(){
		StartCoroutine(Main.Instance.updater.Load<ChargePanel>(
			"Prefabs/ChargePanel",Main.Instance.RootPanel));
	}
	
	public void OnShare(){
		Main.Instance.share.Share("Share App","Hello Player!",cn.sharesdk.unity3d.ContentType.Webpage,"http://www.baidu.com");
	}

	public static bool Dirty=true;
	public static lobby_t Info=null;
	public static IEnumerator ObserveCo(){
		while(true){
			Dirty=true;
			yield return new WaitForSeconds(10*60);
		}
	}
}
