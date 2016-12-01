using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using Proto3;

public class LoadingPanel : MonoBehaviour {
	public GameObject	children;
	public InputField	Host;
	public Text			DefaultHost;

	public Text			percentage;
	public Slider		fileProgress;
	public Slider		totalProgress;

	public static LoadingPanel Instance=null;
	void Awake(){
		Instance=this;
	}
	void OnDestroy(){
		if(Instance==this)
			Instance=null;
	}


	public IEnumerator Process(){
		children.SetActive(true);
		//load host from cache
		Host.text=Config.ws;

		/*
		update & login
		login:
			if cached account: login with it
			else: login with udid
		if updated and logged in: enter lobby
		start account coroutine:
			if not account disable: sign in,cache and update to server
		 */

		fileProgress.value = 0;
		
		//foreach(object o in Game.ToEnumerable(Upgrade()))yield return o;
		//if(Configs.Testing.skipResourceUpdated>0)yield break;
		
		if(DownloadManager.Instance!=null)
			DestroyImmediate(DownloadManager.Instance.gameObject);
		yield return null;
		
		if(!string.IsNullOrEmpty(Config.updateUri)){
			DownloadManager.SetManualUrl(Config.updateUri+"$(Platform)");
		}
		
		while(!DownloadManager.Instance.ConfigLoaded)yield return null;
		
		var bundleInProgress=new List<string>();
		var bundleProgress=new List<string>();
		var lowPriority=new List<string>();
		
		foreach (BundleData bd in DownloadManager.Instance.BuiltBundles) {
			string url = Updater.MakeUrl(bd.name);
			if(DownloadManager.Instance.IsBundleCached(url))continue;
			Debug.Log ("Resource Updating: " + url);
			if(bd.priority>=9){
				bundleInProgress.Add(url);
				bundleProgress.Add(url);
			}else
				lowPriority.Add(url);
		}
		
		int totalInProgress=bundleInProgress.Count;
		if(bundleInProgress.Count+lowPriority.Count>0){
			Debug.Log("need download");
		}
		
		var tm=Time.realtimeSinceStartup;
		while(!Caching.ready&&Time.realtimeSinceStartup-tm<5f)yield return null;
		
		foreach(string url in bundleInProgress)DownloadManager.Instance.StartDownload (url,9);
		
		if(bundleProgress.Count>0)state=progressString(0,totalInProgress,totalInProgress);
		
		while(bundleInProgress.Count>0){
			fileProgress.value = DownloadManager.Instance.ProgressOfBundles (bundleProgress.ToArray ());
			foreach(string url in bundleInProgress){
				var www=DownloadManager.Instance.GetWWW(url);
				if(www!=null&&www.isDone){
					string resname=Updater.MakeName(url);
					Main.Instance.updater.AddResource(resname,www);
					bundleInProgress.Remove(url);
					state=progressString(fileProgress.value,bundleInProgress.Count,totalInProgress);
					Debug.Log("----Asset updated "+resname);
					break;
				}
			}
			state=progressString(fileProgress.value,bundleInProgress.Count,totalInProgress);
			yield return null;
		}
		
		state = "";
		
		foreach(string url in lowPriority)
			DownloadManager.Instance.StartDownload (url);
	}

	public void OnLogin(){
		//choice default if empty
		if(Host.text.Length<=0)Host.text=DefaultHost.text;
		//caching
		if(Host.text.Length<=0){
			Debug.LogError("Invalid host");
			return;
		}

		var uri=Host.text;
		var ws=Host.text;
		if(uri.IndexOf(':')<=0){
			uri+=":8800";
			ws+=":8820";
		}
		if(uri.IndexOf('/')<=0){
			uri="http://"+uri;
			ws="ws://"+ws;
		}
		Config.uri=uri;
		Config.ws=ws;

		if(Main.Instance.GameMode==Main.Mode.STANDALONE){
			//only for testing
			MahJongPanel.Create(delegate(Component obj){
				var gp=obj as GamePanel;
				var msg=gp.Rule.Deal();

				gp.StartCoroutine(gp.OnMsgDeal(Main.Instance.MainPlayer,msg));
				Destroy(gameObject);
			});
		}else if(Main.Instance.GameMode==Main.Mode.NODE)
			DoLogin();
		else{
			MsgCSLogin msg=new MsgCSLogin();
			msg.Mid=pb_msg.MsgCsLogin;
			msg.Version=100;
			msg.User=new user_t();
			msg.User.Account="Unity";
			msg.User.DevType=pb_enum.DevPc;
			msg.User.Name="vic";
			msg.User.Udid=SystemInfo.deviceUniqueIdentifier;
			
			//Debug.Log("----DoLogin account="+msg.User.Account);
			Main.Instance.MainPlayer.http.SetUri(Config.uri);
			Main.Instance.MainPlayer.http.Request<MsgCSLogin>(msg.Mid,msg);
		}
	}

	public void DoLogin(){
		StartCoroutine(Main.Instance.updater.Load<LobbyPanel>(
			"Prefabs/LobbyPanel",Main.Instance.RootPanel,delegate(Object arg1, Hashtable arg2){
			Destroy(gameObject);
		}));
	}
	
	string state{
		set{

		}
	}
	string progressString(float percent,int bundleInProgress,int totalInProgress){
		var percentage=string.Format("{0:#0.}%",100.0f*percent);
		var str=string.Format("下载...{0}",percentage);
		if(Debug.isDebugBuild)str+=string.Format(" ({0}/{1})",totalInProgress-bundleInProgress,totalInProgress);
		//Debug.Log("===="+str);
		return str;
	}

	public void OnUpgrade(){
		var storeUrl="";
		if (Application.platform == RuntimePlatform.IPhonePlayer)
			Application.OpenURL(storeUrl);
		else
			Application.OpenURL(storeUrl);
	}
}
