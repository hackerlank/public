using UnityEngine;
using System.Collections;
using System;  
using System.Collections.Generic;  
using System.IO;  
using System.Text;
using System.Reflection;

namespace com.alipaysdk
{
	public class AlipaySDK : MonoBehaviour 
	{
		public delegate void EventHandler (
			string app_id,string seller_id,string out_trade_no,uint total_amount);

		public EventHandler payHandler;
		public AlipaySDKImpl sdkImp;

		void Awake()
		{				
			//create implements
			#if UNITY_ANDROID
			sdkImp = new AlipaySDKAndroidImpl(gameObject);
			#elif UNITY_IPHONE
			sdkImp = new AlipaySDKiOSImpl(gameObject);
			#endif
		}
		
		public void Order(string orderId,uint amount){
			//request scheme and order string from server
		}
		
		public void Pay(string appScheme,string orderString){
			//invoke payment api
			if(Application.platform != RuntimePlatform.IPhonePlayer&&Application.platform != RuntimePlatform.Android)
				return;

			sdkImp.Pay(appScheme,orderString);
		}

		private void _AliCallback (string data)
		{
			if (data == null) 
			{
				Debug.Log("Alipay returned null");
				return;
			}

			Debug.Log("Alipay returned: "+data);
			Hashtable res = (Hashtable) MiniJSON.jsonDecode(data);
			if (res == null || res.Count <= 0) 
			{
				return;
			}
			
			//int status = Convert.ToInt32(res["status"]);
			//int reqID = Convert.ToInt32(res["reqID"]);

			//if(payHandler!=null)payHandler.Invoke();
		}
	}
}