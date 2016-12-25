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
	public GameObject	LoginButton;

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

	public IEnumerator Login(){
#if DEVELOPMENT_BUILD || UNITY_EDITOR
		Host.gameObject.SetActive(true);
		LoginButton.SetActive(true);
		//load host from cache
		DefaultHost.text=Config.ws;
#else
		//update & login
		StartCoroutine(loginCo());
#endif
		while(Main.Instance.MainPlayer.msgLCLogin==null)
			yield return null;

		//redirection
		var msg=Main.Instance.MainPlayer.msgLCLogin;
		if(!string.IsNullOrEmpty(msg.Redir)){
			Host.text=msg.Redir;
			Config.uri=msg.Redir;

			StartCoroutine(loginCo());

			while(Main.Instance.MainPlayer.msgLCLogin==null)
				yield return null;
		}

		//override assets uri
		if(!string.IsNullOrEmpty(msg.Assets))
			Config.updateUri=msg.Assets;
	}

	public IEnumerator Process(){
		//update
		if(!Main.Instance.updater.SkipUpdate)
			yield return StartCoroutine(updateCo());

		//ready to enter lobby
		Main.Instance.StartCoroutine(Main.Instance.updater.Load<GameSprites>(
			"Prefabs/GameSprites"));

		yield return StartCoroutine(Main.Instance.updater.Load<LobbyPanel>(
			"Prefabs/LobbyPanel",Main.Instance.RootPanel));

		//start sign up/in
		Main.Instance.gameObject.AddComponent<SignInGame>();

		Destroy(gameObject);
	}

	IEnumerator updateCo(){
		//prepare download list via priority
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

		//nothing to do
		if(bundleInProgress.Count+lowPriority.Count<=0)
			yield break;

		//wait for device caching ready
		var tm=Time.realtimeSinceStartup;
		while(!Caching.ready&&Time.realtimeSinceStartup-tm<5f)yield return null;
		if(!Caching.ready)
			Debug.LogError("device storage full or not writalble");
		
		if(bundleProgress.Count>0){
			children.SetActive(true);
			setProgress(0,bundleProgress.Count,bundleProgress.Count);

			//download
			foreach(string url in bundleInProgress)DownloadManager.Instance.StartDownload (url,9);
			while(bundleInProgress.Count>0){
				var percentage=DownloadManager.Instance.ProgressOfBundles(bundleProgress.ToArray());
				foreach(string url in bundleInProgress){
					var www=DownloadManager.Instance.GetWWW(url);
					if(www!=null&&www.isDone){
						string resname=Updater.MakeName(url);
						Main.Instance.updater.AddResource(resname,www);
						bundleInProgress.Remove(url);
						setProgress(percentage,bundleInProgress.Count,bundleProgress.Count);
						Debug.Log("----Asset updated "+resname);
						break;
					}
				}
				setProgress(percentage,bundleInProgress.Count,bundleProgress.Count);
				yield return null;
			}
		}

		//background download
		foreach(string url in lowPriority)
			DownloadManager.Instance.StartDownload (url);
	}

	IEnumerator loginCo(){
#if DEVELOPMENT_BUILD || UNITY_EDITOR
		//choice default if empty
		if(Host.text.Length<=0)Host.text=DefaultHost.text;
		//caching
		if(Host.text.Length<=0){
			Debug.LogError("Invalid host");
			yield break;
		}
		
		var node=Host.text;
		if(node.IndexOf(':')<=0){
			node+=":8820";
		}
		if(node.IndexOf('/')<=0){
			node="ws://"+node;
		}
		Config.ws=node;
#endif

		//login with cached account OR udid
		var udid=SystemInfo.deviceUniqueIdentifier;
		udid=Utils.string2md5(udid);
		var account=PlayerPrefs.GetString(Cache.PrefsKey_Account,udid);
		
		MsgCLLogin msg=new MsgCLLogin();
		msg.Mid=pb_msg.MsgClLogin;
		msg.Version=uint.Parse(Config.build);
		msg.User=new user_t();
		msg.User.Account=account;
		msg.User.DevType=pb_enum.DevPc;
		msg.User.Udid=udid;
		
		//Debug.Log("----DoLogin account="+msg.User.Account);
		Main.Instance.MainPlayer.http.Request<MsgCLLogin>(msg.Mid,msg);

		var now=Time.unscaledTime;
		while(Main.Instance.MainPlayer.msgLCLogin==null){
			if(Time.unscaledTime-now>5 && !BlockView.Instance.Blocking){
				BlockView.Instance.Blocking=true;
			}
			yield return null;
		}
	}

	void setProgress(float percent,int bundleInProgress,int totalInProgress){
		var str=string.Format("{0:#0.}%",100.0f*percent);
		if(Debug.isDebugBuild)str+=string.Format(" ({0}/{1})",totalInProgress-bundleInProgress,totalInProgress);
		fileProgress.value = (float)(totalInProgress-bundleInProgress)/(float)totalInProgress;
		totalProgress.value = percent;
		percentage.text=string.Format("{0:#0.}%",100.0f*percent);
	}

	public void OnLogin(){
		StartCoroutine(loginCo());
		Host.gameObject.SetActive(false);
		LoginButton.SetActive(false);
	}
	
	public void OnUpgrade(){
		var storeUrl="";
		if (Application.platform == RuntimePlatform.IPhonePlayer)
			Application.OpenURL(storeUrl);
		else
			Application.OpenURL(storeUrl);
	}
}
