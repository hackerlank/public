using UnityEngine;
using System.Collections;
using Proto3;

public interface GameController:PlayerController{
	//game rule
	int Round{get;set;}
	GameRule Rule{get;set;}

	//card operation
	bool CardDrag{get;}
	void TapCard(Card card,bool select=true);
	IEnumerator Discard(Card card=null);

	//ui
	void OnExit();
}
