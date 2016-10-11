using UnityEngine;
using System.Collections;
using Proto3;

public interface GameController:PlayerController{
	//message handler
	IEnumerator	OnMsgStart(MsgNCStart msg);
	IEnumerator	OnMsgDiscard(MsgNCDiscard msg);
	void		OnMsgDraw(MsgNCDraw msg);
	void		OnMsgMeld(MsgNCMeld msg);
	void		OnMsgSettle(MsgNCSettle msg);
	void		OnMsgFinish(MsgNCFinish msg);

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
