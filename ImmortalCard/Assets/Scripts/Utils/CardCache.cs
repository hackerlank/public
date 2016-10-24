using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

public class CardCache{

	public static Dictionary<string,Sprite> sprites=new Dictionary<string, Sprite>();
	public static Dictionary<string,GameObject> cards=new Dictionary<string, GameObject>();
	public static bool Ready=false;

	public static IEnumerator Load(string[] files,string path){
		while(Main.Instance.gameController==null)
			yield return null;
		
		//Debug.Log("----caching "+path);
		Ready=false;
		//load Card.prefab
		if(!cards.ContainsKey(path)||cards[path]==null){
			Utils.Load<Card>(null,delegate(Component comp){
				cards[path]=comp.gameObject;
				cards[path].SetActive(false);
			},path);
			yield return null;
		}
		
		//load all cards Sprite
		foreach(var f in files){
			if(!sprites.ContainsKey(f)){
				yield return null;
				Utils.SpriteCreate(f,delegate(Sprite sprite) {
					if(sprite==null)
						Debug.LogError("load card failed "+f);
					sprites[f]=sprite;
				});
			}
		}
		//waiting
		foreach(var f in files)while(!sprites.ContainsKey(f))yield return null;
		while(!cards.ContainsKey(path)||cards[path]==null)yield return null;
		//Debug.Log("----cached "+path);
		Ready=true;
	}
}

