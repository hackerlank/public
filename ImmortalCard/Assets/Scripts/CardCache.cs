using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

public class CardCache{

	public static Dictionary<string,Sprite> sprites=new Dictionary<string, Sprite>();
	public static GameObject card;
	public static bool Ready=false;
	
	public static IEnumerator Load(){
		while(Main.Instance.gameController==null)
			yield return null;

		Ready=false;
		//load Card.prefab
		if(card==null)
			Utils.Load<Card>(null,delegate(Component comp){
				card=comp.gameObject;
				card.SetActive(false);
			});
		yield return null;

		//load all cards Sprite
		var files=new List<string>();
		files.Add("back");
		files.Add("c14");
		files.Add("d15");
		for(uint j=0;j<4;++j)
			for(uint i=1;i<=13;++i)
				files.Add(Main.Instance.gameController.Id2File(j,i));
		foreach(var f in files){
			yield return null;
			Utils.SpriteCreate(f,delegate(Sprite sprite) {
				//Debug.Log("loaded card file="+f);
				sprites[f]=sprite;
			});
			while(!sprites.ContainsKey(f))yield return null;
		}
		while(card==null)yield return null;
		Ready=true;
	}
}

