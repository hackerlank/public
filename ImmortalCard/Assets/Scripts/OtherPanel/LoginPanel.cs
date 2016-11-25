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
}
