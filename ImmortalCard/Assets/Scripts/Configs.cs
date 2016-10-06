using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

public class Configs{

	public static string uri="http://127.0.0.1:8800";
	public static string ws="ws://127.0.0.1:8820";
		
	public static void Load(){
		string file="Config/config.ini";
		TextAsset text = (TextAsset)Resources.Load(file);
		if(text!=null){
			string[] lines = text.text.Split('\n');
			foreach(var line in lines){
				string[] values = line.Split('=');
				if(values.Length>1){

					System.Type type = typeof(Configs);
					string path = values[0];
					FieldInfo fi = type.GetField(path);
					if(fi==null)fi=type.GetField(path,BindingFlags.Instance|BindingFlags.NonPublic);
					if(fi==null)continue;
					if(fi.FieldType!=typeof(string))continue;
					fi.SetValue(null,values[1]);
				}
			}
		}
		Debug.Log("uri="+uri);
		Debug.Log("ws="+ws);
	}
}

