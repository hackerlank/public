using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace com.alipaysdk
{
	#if UNITY_IPHONE
	public class AlipaySDKiOSImpl : AlipaySDKImpl
	{
		[DllImport("__Internal")]
		private static extern void __iosAlipaySDKPay (string appScheme,string orderString,string objectName);

		private string _callbackObjectName = "Main Camera";

		public AlipaySDKiOSImpl (GameObject go) {
			try{
				_callbackObjectName = go.name;
			} catch(Exception e) {
				Console.WriteLine("{0} Exception caught.", e);
			}
		}

		public override void Pay(string appScheme,string orderString){
			//invoke payment api
			if(Application.platform != RuntimePlatform.IPhonePlayer)
				return;
			__iosAlipaySDKPay(appScheme,orderString,_callbackObjectName);
		}
	}
	#endif
}