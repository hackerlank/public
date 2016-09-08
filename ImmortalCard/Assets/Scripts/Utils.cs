using UnityEngine;
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
}
