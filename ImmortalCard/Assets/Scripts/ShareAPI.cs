using UnityEngine;
using System.Collections;
using cn.sharesdk.unity3d;

public class ShareAPI {
	ShareSDK _sdk;

	public ShareAPI() {
		_sdk=Main.Instance.gameObject.GetComponent<ShareSDK>();
		if(_sdk==null || Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)
			return;

		_sdk.appKey=Config.modId;
		_sdk.devInfo.wechat.Enable=true;
		_sdk.devInfo.wechatMoments.Enable=true;
		//sdk.devInfo.wechatFavorites.Enable=true;
		//sdk.devInfo.wechatSeries.Enable=true;

		_sdk.shareHandler = ShareResultHandler;
		_sdk.authHandler = AuthResultHandler;
		_sdk.showUserHandler = GetUserInfoResultHandler;
	}

	public void SignIn(){
		if(_sdk==null || Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)
			return;
		
		_sdk.Authorize(PlatformType.WeChat);
		_sdk.GetUserInfo(PlatformType.WeChat);
	}

	public void Share(string title,string text,int type,string url=""){
		//type[ ContentType.Image=2, ContentType.Webpage=4, ContentType.App=7 ]
		if(_sdk==null || Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)
			return;

		ShareContent content = new ShareContent();
		content.SetTitle(title);
		content.SetText(text);
		//content.SetImageUrl("http://www.mob.com/images/logo_black.png");
		content.SetImagePath(PrepareShareImage(type==ContentType.Image));
		content.SetUrl(url);
		content.SetSiteUrl(url);
		content.SetShareType(type);

		_sdk.ShowPlatformList(null, content, 100, 100);
	}

	void ShareResultHandler (int reqID, ResponseState state, PlatformType type, Hashtable result){
		if (state == ResponseState.Success)
		{
			Debug.Log("share result :");
			Debug.Log(MiniJSON.jsonEncode(result));
		}
		else if (state == ResponseState.Fail)
		{
			Debug.Log("share fail! error code = " + result["error_code"] + "; error msg = " + result["error_msg"]);
		}
		else if (state == ResponseState.Cancel) 
		{
			Debug.Log("share cancel !");
		}
		else
			Debug.Log("share unknown !");
	}

	void AuthResultHandler(int reqID, ResponseState state, PlatformType type, Hashtable result){
		if (state == ResponseState.Success)
		{
			Debug.Log("authorize success !");
		}
		else if (state == ResponseState.Fail)
		{
			Debug.Log("authorize fail! error code = " + result["error_code"] + "; error msg = " + result["error_msg"]);
		}
		else if (state == ResponseState.Cancel) 
		{
			Debug.Log("authorize cancel !");
		}
		else
			Debug.Log("authorize unknown");
	}

	void GetUserInfoResultHandler (int reqID, ResponseState state, PlatformType type, Hashtable result){
		if (state == ResponseState.Success)
		{
			Debug.Log("get user info result :");
			Debug.Log(MiniJSON.jsonEncode(result));
		}
		else if (state == ResponseState.Fail)
		{
			Debug.Log("get user info fail! error code = " + result["error_code"] + "; error msg = " + result["error_msg"]);
		}
		else if (state == ResponseState.Cancel) 
		{
			Debug.Log("get user info cancel !");
		}
		else
			Debug.Log("get user info unknown");
	}

	string PrepareShareImage(bool capture){
		string imagePath="";
		if(capture){
			Application.CaptureScreenshot("screenshot.png");
			imagePath=Application.persistentDataPath + "/screenshot.png";
		}else{
			//should be in the root of Resources folder
			var file="immortal";
			imagePath = Application.persistentDataPath + "/"+file+".png";  
			if (!System.IO.File.Exists(imagePath)){  
				Texture2D o = Resources.Load(file) as Texture2D;
				if (o != null)
					System.IO.File.WriteAllBytes(imagePath, o.EncodeToPNG());
			}
		}
		return imagePath;
	}
}
