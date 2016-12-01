using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public const int		Round=2;
	public StoreGame		storeGame;	//store game for reconnect

	public GameController	gameController=null;
	public ShareAPI			share;

	public Transform		RootPanel;
	public Animator			spinner;
	public GameObject		loginPanel;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	void Awake(){
		Instance=this;

		//init components
		gameObject.AddComponent<Loom>();	//Loom
		updater=gameObject.AddComponent<Updater>();
	}

	IEnumerator Start () {
		//load config
		TextAsset text = (TextAsset)Resources.Load(Configs.file);
		if(text!=null)
			Configs.Load(text.text);

		//update config
		yield return StartCoroutine(updater.Load<TextAsset>(
			"Config/config",null,delegate(Object obj,Hashtable param){
			var ta=obj as TextAsset;
			Configs.Load(ta.text);
		}));

		//force update
		var forceUpdate=(int.Parse(Configs.update)!=0);
		if(forceUpdate){
			var storeUrl="";
			if (Application.platform == RuntimePlatform.IPhonePlayer)
				storeUrl="";
			Application.OpenURL(storeUrl);
			yield break;
		}

		//update LoginPanel and reload
		yield return StartCoroutine(updater.Load<LoginPanel>(
			"Prefabs/LoginPanel",RootPanel,delegate(Object arg1, Hashtable arg2) {
			var panel=arg1 as LoginPanel;
			panel.StartCoroutine(panel.Process());
		}));

		if(loginPanel!=null)
			Destroy(loginPanel);

		//init
		MainPlayer=new Player();
		MainPlayer.playData=new Proto3.play_t();
		MainPlayer.playData.Player=new Proto3.player_t();
		MainPlayer.playData.Player.Uid=SystemInfo.deviceUniqueIdentifier;

		Application.targetFrameRate = 30;
		//Application.backgroundLoadingPriority = ThreadPriority.High;
		//Application.runInBackground = true;
		//Screen.orientation=ScreenOrientation.Portrait;
		share=new ShareAPI();
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

	Updater updater;
	public Updater resourceUpdater{
		get{
			return updater;
		}
	}

	public bool Wait{
		get{
			return Main.Instance.spinner.gameObject.activeSelf;
		}
		set{
			Main.Instance.spinner.gameObject.SetActive(value);
		}
	}

	public void StartWait(float t=5){
		if(!waiting){
			waiting=true;
			StartCoroutine(waitCo(t));
		}
	}
	public void StopWait(){
		StopCoroutine(waitCo(0));
		Wait=false;
		waiting=false;
	}

	bool waiting=false;
	IEnumerator waitCo(float t){
		yield return new WaitForSeconds(t);
		Wait=true;
	}

	public Player MainPlayer=new Player();
	public List<Player> robots=new List<Player>();
}
