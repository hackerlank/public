using UnityEngine;
using System.Collections;

public class GameSprites : IconSprites {
	public static GameSprites Instance=null;
	
	override protected void Awake(){
		base.Awake();
		Instance=this;
	}
	
	void OnDestroy(){
		Instance=null;
	}
}
