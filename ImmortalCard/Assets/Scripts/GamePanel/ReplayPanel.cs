using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ReplayPanel : GamePanel {
	public Transform[]	MeldAreas;		//MROL(Me,Right,Opposite,Left)
	public Transform[]	AbandonAreas;	//MROL(Me,Right,Opposite,Left)
	public GameObject	BtnPlay,BtnPause,BtnStop,BtnPre,BtnNext;
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();

		btnOps=new GameObject[]{BtnPlay,BtnPause,BtnStop,BtnPre,BtnNext};
	}

	public IEnumerator Play(MsgLCReplay msg){
		maxPlayer=msg.Hands.Count;
		_pos=0;
		transformComponent(DiscardAreas);
		transformComponent(HandAreas);
		transformComponent(Players);
		transformComponent(nHandCards);

		//deal
		for(int i=0;i<maxPlayer;++i){
			var hands=msg.Hands[i].Pawns;
			int fin=0;
			foreach(var id in hands){
				Card.Create(CardPrefab,id,HandAreas[i],delegate(Card card) {
					++fin;
				});
			}
			while(fin<hands.Count)yield return null;
		}

		//engage

		//replay
		foreach(bunch_t op in msg.Ops){
			yield return new WaitForSeconds(0.5f);
			int fin=0;
			Transform trans=null;

			switch(op.Type){
			case pb_enum.OpPass:
				trans=AbandonAreas[op.Pos];
				break;
			default:
				trans=MeldAreas[op.Pos];
				break;
			}

			foreach(var id in op.Pawns){
				Card.Create(CardPrefab,id,trans,delegate(Card card) {
					++fin;
				});
			}
			while(fin<op.Pawns.Count)yield return null;
		}
		yield break;
	}

	override public string Id2File(int color,int value){return "";}
	override public float DiscardScalar{get{return 1;}}
	override public string CardPrefab{get{return "Zipai";}}
	override public void PrepareCache(){}

	public static void Create(string path,System.Action<Component> handler=null){
		Utils.Load<ReplayPanel>(Main.Instance.RootPanel,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		},path);
	}
}
