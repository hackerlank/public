using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class IconSprites : MonoBehaviour {

	public Sprite[] sprites;
	
	[HideInInspector]
	public Dictionary<pb_enum,Sprite>	dict;
	
	virtual protected void Awake(){
		//build sprites dictionary
		dict=new Dictionary<pb_enum, Sprite>();
		foreach(var sprite in sprites){
			if(null==sprite)continue;
			
			var name=sprite.name;
			var j=name.LastIndexOf("_")+1;
			var prefix=name.Substring(j);
			var key=(pb_enum)int.Parse(prefix);
			dict[key]=sprite;
		}
	}
}
