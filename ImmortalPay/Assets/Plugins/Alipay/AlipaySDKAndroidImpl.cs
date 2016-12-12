using System;
using System.Collections;
using UnityEngine; 

namespace com.alipaysdk
{
	#if UNITY_ANDROID
	public class AlipaySDKAndroidImpl : AlipaySDKImpl
	{
		//private AndroidJavaObject ssdk;

		public AlipaySDKAndroidImpl (GameObject go) 
		{
			/*
			try{
				ssdk = new AndroidJavaObject("cn.sharesdk.unity3d.ShareSDKUtils", go.name, "_Callback");
			} catch(Exception e) {
				Console.WriteLine("{0} Exception caught.", e);
			}
			*/
		}

		public override void Pay(string appScheme,string orderString){
			//invoke payment api
			if(Application.platform != RuntimePlatform.Android)
				return;
			/*
			if (ssdk != null) 
			{			
				ssdk.Call("initSDK", appKey);
			}
			*/
		}
	}
	#endif
}
