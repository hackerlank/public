using System;
using System.Collections;
using UnityEngine; 

namespace com.alipaysdk
{
	#if UNITY_ANDROID
	public class AlipaySDKAndroidImpl : AlipaySDKImpl
	{
		string _objName;
		public AlipaySDKAndroidImpl (GameObject go) 
		{
			_objName=go.name;
		}

		public override void Pay(string appScheme,string orderString){
			//invoke payment api
			if(Application.platform != RuntimePlatform.Android)
				return;

			using (AndroidJavaClass unity_player = new AndroidJavaClass("com.unity3d.player.UnityPlayer"))
			{
				using (AndroidJavaObject current_activity = unity_player.GetStatic<AndroidJavaObject>("currentActivity"))
				{
					AndroidJavaObject ssdk=null;
					try{
						ssdk = new AndroidJavaObject("com.immorplay.pay.alipayBridge");
					} catch(Exception e) {
						Debug.LogError("Exception caught: "+e);
					}

					if (ssdk != null) 
					{
						Debug.Log("call alipay API");
						ssdk.Call("__alipayBridgePay",current_activity,orderString,_objName,"_AliCallback");
					}
					else
						Debug.LogError("alipayBridge null");
				}
			}
		}
	}
	#endif
}
