using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;

public class Utils {

	public static void Load<T>(Transform parent=null,System.Action<Component> action=null){
		string url="Prefabs/"+typeof(T).ToString();
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

	public static IEnumerator LoadAsync<T>(Transform parent=null){
		string url="Prefabs/"+typeof(T).ToString();
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

	public static void SpriteCreate(string name,System.Action<Sprite> handler=null,string path="Cards/"){
		string url=path+name;
		var obj=Resources.Load(url,typeof(Sprite));
		Sprite sprite=null;
		if(obj)sprite=MonoBehaviour.Instantiate(obj) as Sprite;
		if(handler!=null)handler.Invoke(sprite);
	}
	
	public static void ImageReset(Image img,string name,bool resize=false){
		SpriteCreate(name,delegate(Sprite sp) {
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
}
