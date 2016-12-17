using UnityEngine;
using System.Collections;

public class RuleSprites : IconSprites {
	public static RuleSprites Instance=null;
	
	override protected void Awake(){
		base.Awake();
		Instance=this;
	}
	
	void OnDestroy(){
		Instance=null;
	}
}
