using UnityEngine;
using System.Collections;
using Proto3;

public interface GameController:PlayerController{
	//game rule
	uint Round{get;set;}
	GameRule Rule{get;set;}

	//card operation
	void TapCard(Card card,bool select=true);
	IEnumerator Discard(Card card=null);

	//ui
	void OnExit();
	string Id2File(uint color,uint value);
	float DiscardScalar{get;}
}
