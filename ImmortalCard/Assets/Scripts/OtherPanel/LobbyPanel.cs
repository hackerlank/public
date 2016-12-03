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
			//in game: reconnect and create game panel,robots
			Debug.Log("found game "+storeGame.gameId);
			Cache.storeGame=storeGame;
			yield return StartCoroutine(Main.Instance.MainPlayer.Reconnect());

			Destroy(gameObject);
		}else{
			//request and show lobby
			var old=lobby;
			if(Dirty){
				MsgCLLobby msg=new MsgCLLobby();
				msg.Mid=pb_msg.MsgClLobby;
				msg.Version=(old==null?0:old.Version);
				msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
				Main.Instance.MainPlayer.http.Request<MsgCLLobby>(msg.Mid,msg);
				
				lobby=null;
			}
			
			while(lobby==null)
				yield return null;
			
			if(old!=null && lobby.Version<=old.Version)
				lobby=old;
			
			//bulletin
			if(!string.IsNullOrEmpty(lobby.Bulletin)){
				Bulletin.text=lobby.Bulletin;
			}
			
			//games
			foreach(game_t game in lobby.Games){
				Hashtable param=new Hashtable();
				param["game"]=game;
				StartCoroutine(Main.Instance.updater.Load<GameIcon>(
					"Prefabs/GameIcon",GameRoot,delegate(Object obj,Hashtable arg){
					var icon=obj as GameIcon;
					icon.Value=arg["game"] as game_t;
				},param));
				yield return null;
			}
		}
	}

	public void OnGame(game_t game){
		StartCoroutine(Main.Instance.updater.Load<EnterPanel>(
			"Prefabs/EnterPanel",Main.Instance.RootPanel,delegate(Object arg1, Hashtable arg2) {
			var panel=arg1 as EnterPanel;
			panel.CurrentGame=game;
			if(null!=this)
				Destroy(gameObject);
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
	public static lobby_t lobby=null;
	public static IEnumerator ObserveCo(){
		while(true){
			Dirty=true;
			yield return new WaitForSeconds(10*60);
		}
	}
}
