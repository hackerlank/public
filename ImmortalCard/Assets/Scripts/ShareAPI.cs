using UnityEngine;
using System.Collections;
using cn.sharesdk.unity3d;

public class ShareAPI {
	ShareSDK sdk;

	public ShareAPI() {
		if(Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)return;

		sdk=Main.Instance.gameObject.GetComponent<ShareSDK>();
		sdk.appKey=Configs.modId;
		sdk.devInfo.wechat.Enable=true;
		sdk.devInfo.wechatMoments.Enable=true;
		sdk.devInfo.wechatFavorites.Enable=true;
		sdk.devInfo.wechatSeries.Enable=true;

		sdk.shareHandler = ShareResultHandler;
		sdk.authHandler = AuthResultHandler;
		sdk.showUserHandler = GetUserInfoResultHandler;
	}

	public void SignIn(){
		if(Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)return;
		
		sdk.Authorize(PlatformType.SinaWeibo);
		sdk.GetUserInfo(PlatformType.SinaWeibo);
	}
	
	public void Share(string title,string text){
		if(Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)return;
		
		ShareContent content = new ShareContent();
		content.SetText(text);
		content.SetImagePath("Images/Immortal");
		content.SetTitle(title);
		content.SetShareType(ContentType.Image);
		
		//share from menu
		sdk.ShowPlatformList (null, content, 100, 100);
		//share from editor
		sdk.ShowShareContentEditor (PlatformType.WeChat, content);
		//share directly
		sdk.ShareContent (PlatformType.WeChat, content);
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
}
