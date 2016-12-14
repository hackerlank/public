using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public const int		Round=2;

	[HideInInspector]
	public Updater			updater;
	public GameController	gameController=null;
	public ShareAPI			share;

	public Transform		RootPanel;
	public GameObject		tempLoadingPanel;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NORMAL;}}

	void Awake(){
		Instance=this;

		//init components
		gameObject.AddComponent<Loom>();	//Loom
		updater=gameObject.AddComponent<Updater>();
	}

	IEnumerator Start () {
		//local->server->(redirection->)bundles
		//load config
		TextAsset text = (TextAsset)Resources.Load(Config.file);
		if(text!=null)
			Config.Load(text.text);

		//TODO: update uri should come from server
		//we could not use uri from local or downloading
		//remove Config.updateUri
		if(!string.IsNullOrEmpty(Config.updateUri)){
			if(!Config.updateUri.EndsWith("/"))
				Config.updateUri+="/";
			DownloadManager.SetManualUrl(Config.updateUri+"$(Platform)");
		}
		while(!DownloadManager.Instance.ConfigLoaded)yield return null;

		//update config
		yield return StartCoroutine(updater.Load<TextAsset>(
			"Config/config",null,delegate(Object obj,Hashtable param){
			var ta=obj as TextAsset;
			Config.Load(ta.text);
		}));
		MainPlayer.http.SetUri(Config.uri);
		MainPlayer.playData=new Proto3.play_t();

		//force update
		var forceUpdate=(int.Parse(Config.update)!=0);
		if(forceUpdate){
			var storeUrl="";
			if (Application.platform == RuntimePlatform.IPhonePlayer)
				storeUrl="";
			Application.OpenURL(storeUrl);
			yield break;
		}

		//update LoadingPanel and reload
		yield return StartCoroutine(updater.Load<LoadingPanel>(
			"Prefabs/LoadingPanel",RootPanel,delegate(Object arg1, Hashtable arg2) {
			var panel=arg1 as LoadingPanel;
			panel.StartCoroutine(panel.Process());
		}));

		if(tempLoadingPanel!=null)
			Destroy(tempLoadingPanel);

		//init
		Application.targetFrameRate = 30;
		//Application.backgroundLoadingPriority = ThreadPriority.High;
		//Application.runInBackground = true;
		//Screen.orientation=ScreenOrientation.Portrait;
		share=new ShareAPI();
		StartCoroutine(LobbyPanel.ObserveCo());
	}
	
	void Update () {
		#if UNITY_EDITOR
		//if(Input.GetKeyUp(KeyCode.F3)){}
		#endif
	}
	void OnApplicationPause(bool pauseStatus) {
		if(pauseStatus){
			Resources.UnloadUnusedAssets();	//anyway need this
		}
	}
	void OnApplicationFocus(bool focusStatus) {
	}
	
	void OnApplicationQuit(){
	}

	public void StartWait(float t=5){
		if(!waiting){
			waiting=true;
			StartCoroutine(waitCo(t));
		}
	}
	public void StopWait(){
		StopCoroutine(waitCo(0));
		BlockView.Instance.Blocking=false;
		waiting=false;
	}

	bool waiting=false;
	IEnumerator waitCo(float t){
		yield return new WaitForSeconds(t);
		BlockView.Instance.Blocking=true;
	}

	public Player MainPlayer=new Player();
	public List<Player> robots=new List<Player>();
}
