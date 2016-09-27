using UnityEngine;
using System.Collections;
using Proto3;

public interface GameController {
	//message handler
	IEnumerator	Deal(MsgNCStart msg);
	IEnumerator	OnDiscardAt(MsgNCDiscard msg);
	void		OnSettle(MsgNCSettle msg);
	void		OnFinish(MsgNCFinish msg);

	//game rule
	uint Round{get;set;}
	GameRule Rule{get;set;}

	//card operation
	void OnCard(Card card,bool select=true);
	void Discard(Card card=null);

	//ui
	void OnExit();
}
