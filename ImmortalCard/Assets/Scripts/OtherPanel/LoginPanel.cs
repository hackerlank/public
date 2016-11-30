using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.IO;
using Proto3;

public class LoginPanel : MonoBehaviour {

	public InputField Host;
	public Text DefaultHost;

	public static LoginPanel Instance=null;
	void Awake(){
		//load host from cache
		Host.text=PlayerPrefs.GetString(Configs.PrefsKey_Uri);
		Instance=this;
	}
	void OnDestroy(){Instance=null;}
	
	public void OnLogin(){
		//choice default if empty
		if(Host.text.Length<=0)Host.text=DefaultHost.text;
		//caching
		if(Host.text.Length>0){
			var saved=PlayerPrefs.GetString(Configs.PrefsKey_Uri);
			if(Host.text!=saved){
				PlayerPrefs.SetString(Configs.PrefsKey_Uri,Host.text);
				PlayerPrefs.Save();
			}
		}else{
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
		Configs.uri=uri;
		Configs.ws=ws;

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
			Main.Instance.MainPlayer.http.SetUri(Configs.uri);
			Main.Instance.MainPlayer.http.Request<MsgCSLogin>(msg.Mid,msg);
		}
	}

	public void DoLogin(){
		//Main.Instance.share.SignIn();
		Utils.Load<LobbyPanel>(Main.Instance.RootPanel,delegate(Component obj){
			Destroy(gameObject);
		});
	}

	/*
	private IEnumerator Process(){
		animator.SetBool ("IsOpen",true);
		StartCoroutine (StartMusic ());
		//float startTime = Time.time;
		if (!Configs.EndlessMode) {
			slider.value = 0;
			state = Game.Localize(Msg.STARTUP_STATUS_CONNECTING.ToString());
			animator.SetTrigger("StatusChanged");
			animator.SetBool("StatusIsShown",true);
			foreach(object o in Game.ToEnumerable(StartUp.Instance.Login()))yield return o;
			foreach(object o in Game.ToEnumerable(ResendReliableMessages()))yield return o;
			if(ResentReliableMessages){
				ResentReliableMessages = false;
				foreach(object o in Game.ToEnumerable(StartUp.Instance.Login()))yield return o;
			}
			Misc.LogProgress("Connected");
			state = Game.Localize(Msg.STARTUP_STATUS_CHECKINGFORUPDATES.ToString());
			animator.SetTrigger("StatusChanged");
			foreach(object o in Game.ToEnumerable(Upgrade()))yield return o;

			if(Configs.Testing.skipResourceUpdated>0)yield break;

			if(DownloadManager.Instance!=null)DestroyImmediate(DownloadManager.Instance.gameObject);
			yield return null;

			Misc.LogProgress("BeginUpdate");
			var fakeMsg=new Message_T(1,null);
			Game.instance.httpHelper.Listener.OnSend(fakeMsg);
			if(!string.IsNullOrEmpty(Configs.Testing.overrideResourceUpdaterAddress)){
				DownloadManager.SetManualUrl(Configs.Testing.overrideResourceUpdaterAddress+"$(Platform)");
			}else{
				string path = Game.instance.netHandler.globalConfigs.resourceUpdaterPath;
				if(string.IsNullOrEmpty(path))path = Configs.S_APP_URL +"assets/";
				DownloadManager.SetManualUrl(path+"$(Platform)");
				showDebug(path);
			}

			DownloadManager.Instance.ErrorHandler=delegate(string error) {
				StatsHelper.Log(StatsType.ASSETS_ERROR,new Dictionary<string, object>(){{"assets","BMData"},{"message",error}});
				Misc.ShowNormalErrorMessage(Game.Localize(Msg.ASSETS_ERROR_CONTENT),Game.Localize(Msg.ASSETS_ERROR_TITLE),error,delegate(BasePopup obj) {
					if(obj!=null)obj.OnCancel();
					Game.StartCo(RestartAfterClosingPopups());
				});
			};
			while(!DownloadManager.Instance.ConfigLoaded)yield return CDbg.Null();
			Game.instance.httpHelper.Listener.OnRecv("",null,null,null,fakeMsg);
			StartCoroutine(downloadAssetsVersion());

			StatsHelper.Log(StatsType.RESOURCEUPDATE_RESPONSE);
			string urlAssets=DownloadManager.Instance.downloadRootUrl;
			showDebug(urlAssets);

			List<string> bundleInProgress=new List<string>(),bundleProgress=new List<string>()
				,lowPriority=new List<string>();

			//BundleData wbd = DownloadManager.Instance.AddTempBundle("World");
			//BundleData ctbd = DownloadManager.Instance.AddTempBundle("CreatureTypeList");

			foreach (BundleData bd in DownloadManager.Instance.BuiltBundles) {
				string url = DeltaResourceUpdater.MakeUrl(bd.name);
				Game.instance.resourceUpdater.AddServerResource(bd.name,url);
				if(url.Contains("WarnutsAudio")||url.Contains("MasterAudioPlaylistControllers")){
					StartUp.Instance.NeedToReloadAudio = true;
				}
				if(DownloadManager.Instance.IsBundleCached(url,bd.name))continue;
				//Ndbg.Log ("Resource Updating: " + url);
				if(bd.priority>=9||
				   url.Contains("content")||
				   url.Contains("WarnutsLanguages")||
				   url.Contains("MainCamera")||
				   url.Contains("firedragon_01")||
				   url.Contains("mermaidqueen_01")||
				   url.Contains("monkey_01")||
				   url.Contains("mouse_01")||
				   url.Contains("pinocchio_01")||
				   url.Contains("WarnutsAudio")){
					bundleInProgress.Add(url);
					bundleProgress.Add(url);
				}else
					lowPriority.Add(url);
			}
			 
			int totalInProgress=bundleInProgress.Count;
			if(bundleInProgress.Count+lowPriority.Count>0){
				animator.SetTrigger("StatusChanged");
				animator.SetBool("IsUpdatingContent",true);
				StatsHelper.Log(StatsType.RESOURCEUPDATE_NEEDED);
			}

			var tm=Time.realtimeSinceStartup;
			while(!Caching.ready&&Time.realtimeSinceStartup-tm<5f)yield return CDbg.Null();

			foreach(string url in bundleInProgress)DownloadManager.Instance.StartDownload (url,9);
			StartedResourceUpdating = true;
			if(bundleProgress.Count>0)state=progressString(0,totalInProgress,totalInProgress);

			bool over25 = false,over50 = false,over75 = false;
			while(bundleInProgress.Count>0){
				slider.value = DownloadManager.Instance.ProgressOfBundles (bundleProgress.ToArray ());
				foreach(string url in bundleInProgress){
					var www=DownloadManager.Instance.GetWWW(url);
					if(www!=null&&www.isDone){
						string resname=DeltaResourceUpdater.MakeName(url);
						Game.instance.resourceUpdater.AddResource(resname,www);
						bundleInProgress.Remove(url);
						//state=progressString(slider.value,bundleInProgress.Count,totalInProgress);
						animator.SetTrigger("StatusChanged");
						Debug.Log("----Asset updated "+resname);
						break;
					}
				}
				state=progressString(slider.value,bundleInProgress.Count,totalInProgress);
				if((!over25)&&(slider.value>=0.25f)){
					over25 = true;
					StatsHelper.Log(StatsType.RESOURCEUPDATE_25);
				}
				if((!over50)&&(slider.value>=0.50f)){
					over50 = true;
					StatsHelper.Log(StatsType.RESOURCEUPDATE_50);
				}
				if((!over75)&&(slider.value>=0.75f)){
					over75 = true;
					StatsHelper.Log(StatsType.RESOURCEUPDATE_75);
				}
				//Debug.Log("----progress of download="+slider.value.ToString()+","+downloadedAlready.ToString()+" of "+bundleProgress.Count.ToString());
				yield return CDbg.Null();
			}
			if(over25)StatsHelper.Log(StatsType.RESOURCEUPDATE_100);
			animator.SetBool("IsUpdatingContent",false);
			state = "";
			DownloadManager.Instance.ErrorHandler=null;

			foreach(string url in lowPriority)
				DownloadManager.Instance.StartDownload (url);
		}
		//while ((Time.time-startTime)<Configs.HLPanelConfigs.MinUpdateScreenShowTime)yield return CDbg.Null();

		List<string> cachelist=new List<string>();
		foreach(var team in Game.instance.player.info.teams.team){
			foreach(var id in team.creaturesIds)
				if(id>0)foreach(CreatureInstance c in Game.instance.player.info.creaturebox)
					if(c.cid==id)cachelist.Add("Sprite/Creature/" +c.slug);
		}
		Game.instance.resourceUpdater.cache(cachelist.ToArray(),8);
		Misc.LogProgress("EndUpdate");
	}

	string progressString(float percent,int bundleInProgress,int totalInProgress){
		//percent=(float)(totalInProgress-bundleInProgress-1)/(float)totalInProgress;
		//var percentage=string.Format("{0:#0.#}%",100.0f*percent);
		var percentage=string.Format("{0:#0.}%",100.0f*percent);
		var fmt=Game.Localize(Msg.STARTUP_STATUS_UPDATING_0COUNTDONE_1COUNTALL.ToString());
		if(fmt.Contains("/"))fmt=fmt.Substring(0,fmt.IndexOf("/"));
		var str=string.Format(fmt,percentage);
		if(Debug.isDebugBuild)str+=string.Format(" ({0}/{1})",totalInProgress-bundleInProgress,totalInProgress);
		//Debug.Log("===="+str);
		return str;
	}
				if (Application.platform == RuntimePlatform.IPhonePlayer)
				Application.OpenURL (Game.instance.netHandler.globalConfigs.versionSettings.iosUpgradeUrl);
				else
				Application.OpenURL (Game.instance.netHandler.globalConfigs.versionSettings.gpUpgradeUrl);

	 	 */
}
