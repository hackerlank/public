using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

public class Config{
	public static string file="config";

	public static string version="1.0.0";
	public static string build	="100";

	public static string uri="http://127.0.0.1:8880";
	public static string limited="0";	//feature limited: wechat,payment etc.

	//ShareSDK app id
	public static string modId="180127d1c7541";

	public static float OpsInterval=0.5f;
	public static int invalidCard=-1;

	public static void Load(string text){
		if(text!=null){
			var dict=Utils.ParseIni(text);
			foreach(var kv in dict){
				System.Type type = typeof(Config);
				string field = kv.Key;
				FieldInfo fi = type.GetField(field);
				if(fi==null)fi=type.GetField(field,BindingFlags.Instance|BindingFlags.NonPublic);
				if(fi==null)continue;
				if(fi.FieldType!=typeof(string))continue;
				fi.SetValue(null,kv.Value);
			}
		}
	}
}

