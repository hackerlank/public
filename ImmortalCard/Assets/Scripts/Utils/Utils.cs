using UnityEngine;
using UnityEngine.UI;
using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.Security.Cryptography;

public class Utils {

	/*
	public static void Load<T>(Transform parent=null,System.Action<Component> action=null,string path=null){
		string url="Prefabs/"+(path==null?typeof(T).ToString():path);
		GameObject go=Resources.Load(url,typeof(GameObject)) as GameObject;
		if(go){
			go=GameObject.Instantiate(go) as GameObject;
			if(parent!=null)go.transform.SetParent(parent,false);
			//go.transform.localPosition=Vector3.zero;
			//go.transform.localScale=Vector3.one;
			Component t=go.GetComponent(typeof(T));
			if(null!=action)action.Invoke(t);
		}
	}

	public static IEnumerator LoadAsync<T>(Transform parent=null,string path=null){
		string url="Prefabs/"+(path==null?typeof(T).ToString():path);
		ResourceRequest req = Resources.LoadAsync(url,typeof(GameObject));
		yield return req;
		GameObject go=req.asset as GameObject;
		if(go){
			go=GameObject.Instantiate(go) as GameObject;
			if(parent!=null)go.transform.SetParent(parent,false);
			//go.transform.localPosition=Vector3.zero;
			//go.transform.localScale=Vector3.one;
		}
	}
*/
	public static void SpriteCreate(string url,System.Action<Sprite> handler=null){
		var obj=Resources.Load(url,typeof(Sprite));
		Sprite sprite=null;
		if(obj)sprite=MonoBehaviour.Instantiate(obj) as Sprite;
		if(handler!=null)handler.Invoke(sprite);
	}
	
	public static void ImageReset(Image img,string url,bool resize=false){
		SpriteCreate(url,delegate(Sprite sp) {
			img.sprite=sp;
			if(resize)img.rectTransform.sizeDelta=new Vector2(sp.texture.width,sp.texture.height);
		});
	}

	private static DateTime time_t_epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
	public static long time{
		get {
			return (long) (new DateTime(DateTime.UtcNow.Ticks) - time_t_epoch).TotalSeconds;
		}
	}
	
	public static float nanotime{
		get{
			return 0.001f*(float)(System.DateTime.Now.Ticks/10000%100000000);
		}
	}

	public static Vector2 ScreenToUIPoing(Vector2 pt){
		return new Vector2(pt.x-Screen.width/2f,pt.y-Screen.height/2f);
	}

	public static Dictionary<string,string> ParseIni(string text){
		var dict=new Dictionary<string,string>();

		string[] lines = text.Split('\n');
		foreach(var line in lines){
			if(line.StartsWith("#") || line.StartsWith("//"))
				continue;
			string[] values = line.Split('=');
			if(values.Length>1){
				dict[values[0]]=values[1];
			}
		}
		return dict;
	}

	// splits a TSV row
	public static string[] SplitTsvLine(string line)
	{
		return line.Split('\t');
	}
	
	// splits a CSV row 
	public static string[] SplitCsvLine(string line)
	{
		string pattern = @"
	    # Match one value in valid CSV string.
	    (?!\s*$)                                      # Don't match empty last value.
	    \s*                                           # Strip whitespace before value.
	    (?:                                           # Group for value alternatives.
	      '(?<val>[^'\\]*(?:\\[\S\s][^'\\]*)*)'       # Either $1: Single quoted string,
	    | ""(?<val>[^""\\]*(?:\\[\S\s][^""\\]*)*)""   # or $2: Double quoted string,
	    | (?<val>[^,'""\s\\]*(?:\s+[^,'""\s\\]+)*)    # or $3: Non-comma, non-quote stuff.
	    )                                             # End group of value alternatives.
	    \s*                                           # Strip whitespace after value.
	    (?:,|$)                                       # Field ends on comma or EOS.
	    ";
		string[] values = (from Match m in Regex.Matches(line, pattern, 
		                                                 RegexOptions.ExplicitCapture | RegexOptions.IgnorePatternWhitespace | RegexOptions.Multiline)
		                   select m.Groups[1].Value).ToArray();
		return values;      
	}

	public static string byte2string(byte[] bytes){
		var res = new StringBuilder (bytes.Length*2);
		foreach (var b in bytes)
			res.Append (b.ToString ("x2"));
		return res.ToString ();
	}

	public static string string2md5(string src){
		var md5=MD5.Create();
		var bytes = Encoding.UTF8.GetBytes (src);
		return Utils.byte2string(md5.ComputeHash(bytes));
	}
}
