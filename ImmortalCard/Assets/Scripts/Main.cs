using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public int				Round=2;

	[HideInInspector]
	public Updater			updater;
	public GameController	gameController=null;
	public ShareAPI			share;

	public Transform		RootPanel;
	public LoadingPanel		loadingPanel;

	void Awake(){
		Instance=this;

		//init components
		gameObject.AddComponent<Loom>();	//Loom
		updater=gameObject.AddComponent<Updater>();
	}

	IEnumerator Start () {
		/* load config
		 * uri could not be override anywhere
		 * assets uri could be override by server
		 * others include node could be override by assets
		 */
		TextAsset text = (TextAsset)Resources.Load(Config.file);
		if(text!=null)
			Config.Load(text.text);

		//create play data
		MainPlayer.playData=new Proto3.play_t();
		
		//login; get assets uri
		MainPlayer.http.SetUri(Config.uri);
		yield return StartCoroutine(
			loadingPanel.Login()
			);

		//prepare updater
		updater.SkipUpdate=string.IsNullOrEmpty(Config.updateUri);
		if(!updater.SkipUpdate){
			if(!Config.updateUri.EndsWith("/"))
				Config.updateUri+="/";
			DownloadManager.SetManualUrl(Config.updateUri+"$(Platform)");
			while(!DownloadManager.Instance.ConfigLoaded)yield return null;
		}

		//reload BlockView
		DestroyImmediate(BlockView.Instance.gameObject);
		StartCoroutine(Main.Instance.updater.Load<BlockView>(
			"Prefabs/BlockView",Main.Instance.transform));
		while(BlockView.Instance==null)
			yield return null;

		//update config
		yield return StartCoroutine(updater.Load<TextAsset>(
			"Config/config",null,delegate(Object obj,Hashtable param){
			//donnot override uri; updateUri is useless
			var uri=Config.uri;
			var build=int.Parse(Config.build);

			//reload config
			var ta=obj as TextAsset;
			Config.Load(ta.text);

			//restore uri
			Config.uri=uri;

			//check version
			if(build>=int.Parse(Config.build))
				Config.update="0";
		}));

		while(BlockView.Instance==null)
			yield return null;

		//force update
		var forceUpdate=(int.Parse(Config.update)!=0);
		if(forceUpdate){
			BlockView.Instance.ShowDialog(
				"您的版本过低，需要更新才能继续！",
				"",
				delegate {
				var storeUrl=Config.androidMarket;
				if (Application.platform == RuntimePlatform.IPhonePlayer)
					storeUrl=Config.appStore;
				Application.OpenURL(storeUrl);
			});
			yield break;
		}

		//reload LoadingPanel
		yield return StartCoroutine(updater.Load<LoadingPanel>(
			"Prefabs/LoadingPanel",RootPanel,delegate(Object arg1, Hashtable arg2) {
			//destroy old
			if(loadingPanel!=null)
				Destroy(loadingPanel.gameObject);

			loadingPanel=arg1 as LoadingPanel;
		}));

		//update
		loadingPanel.StartCoroutine(loadingPanel.Process());

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
