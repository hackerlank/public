using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using com.alipaysdk;
using Proto3;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public ShareAPI			share;
	public AlipaySDK		alipay;
	public Animator			spinner;
	public Text				version;
	public Text				DefaultUri;
	public InputField		Uri;

	void Awake(){
		Instance=this;
	}

	void Start () {
		//load config
		TextAsset text = (TextAsset)Resources.Load(Config.file);
		if(text!=null)
			Config.Load(text.text);
		version.text="v"+Config.version+"("+Config.build+")";

#if DEVELOPMENT_BUILD || UNITY_EDITOR
		Uri.transform.parent.gameObject.SetActive(true);
		//load host from cache
		DefaultUri.text=Config.uri;
#else
		//update & login
		StartCoroutine(loginCo());
#endif
	}

	public void OnLogin(){
		//choice default if empty
		if(Uri.text.Length<=0)
			Uri.text=DefaultUri.text;

		if(Uri.text.Length>0){
			var uri=Uri.text;
			if(uri.IndexOf(':')<=0){
				uri+=":8880";
			}
			if(uri.IndexOf('/')<=0){
				uri="http://"+uri;
			}
			Config.uri=uri;
		}
		Uri.transform.parent.gameObject.SetActive(false);
		StartCoroutine(loginCo());
	}

	IEnumerator loginCo(){
		if(Uri.text.Length<=0){
			Debug.LogError("empty host");
			yield break;
		}

		Debug.Log("Uri="+Config.uri);
		MainPlayer.http.SetUri(Config.uri);
		MainPlayer.playData=new Proto3.play_t();
		
		Application.runInBackground = true;
		share=new ShareAPI();
		alipay=gameObject.AddComponent<AlipaySDK>();
		
		//login with cached account OR udid
		var udid=SystemInfo.deviceUniqueIdentifier;
		udid=Utils.string2md5(udid);
		var account=PlayerPrefs.GetString(Cache.PrefsKey_Account,udid);
		
		MsgCPLogin msg=new MsgCPLogin();
		msg.Mid=pb_msg.MsgCpLogin;
		msg.Version=uint.Parse(Config.build);
		msg.User=new user_t();
		msg.User.Account=account;
		msg.User.DevType=pb_enum.DevPc;
		msg.User.Udid=udid;
		
		//Debug.Log("----DoLogin account="+msg.User.Account);
		Main.Instance.MainPlayer.http.Request<MsgCPLogin>(msg.Mid,msg);

		var limited=(0==int.Parse(Config.limited));
		if(!limited){
			//if not account disable: sign in,cache and update to server
			yield return new WaitForSeconds(5);
			
			//always sign in to find the new user
			yield return StartCoroutine(Main.Instance.share.SignIn(delegate(string acc) {
				//cache account for next time login
				acc=Utils.string2md5(acc);
				PlayerPrefs.SetString(Cache.PrefsKey_Account,acc);
			}));
		}
		spinner.gameObject.SetActive(false);
		yield break;
	}

	public Player MainPlayer=new Player();
}
