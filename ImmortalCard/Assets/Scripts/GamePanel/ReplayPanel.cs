using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ReplayPanel : GamePanel {
	public GameObject	BtnPlay,BtnPause,BtnStop,BtnPre,BtnNext;
	
	// ----------------------------------------------
	// events
	// ----------------------------------------------
	override public void Awake(){
		base.Awake();

		btnOps=new GameObject[]{BtnPlay,BtnPause,BtnStop,BtnPre,BtnNext};
	}

	public IEnumerator Play(MsgLCReplay msg){
		//prepare cache before panel shown,to ensure revive
		if(!CardCache.Ready){
			Rule.PrepareCache();
			while(!CardCache.Ready)yield return null;
		}

		maxPlayer=msg.Hands.Count;
		_pos=0;
		transformComponent(DiscardAreas);
		transformComponent(HandAreas);
		transformComponent(MeldAreas);
		transformComponent(AbandonAreas);
		transformComponent(Players);
		transformComponent(nHandCards);

		//deal
		for(int i=0;i<maxPlayer;++i){
			var hands=msg.Hands[i].Pawns;
			int fin=0;
			foreach(var id in hands){
				Card.Create(Rule.CardPrefab,id,HandAreas[i],delegate(Card card) {
					card.state=Card.State.ST_MELD;
					++fin;
				});
			}
			while(fin<hands.Count)yield return null;
		}

		//engage

		//replay
		foreach(bunch_t op in msg.Ops){
			yield return new WaitForSeconds(1.5f);
			Debug.Log("----replay "+op.Pos.ToString()+":"+Player.bunch2str(op));

			//remove
			if(op.Type != pb_enum.BunchA)
			foreach(var id in op.Pawns){
				//TODO: optimize violently remove
				for(int i=0;i<maxPlayer;++i){
					//remove from hands
					var removed=false;
					var hands=HandAreas[i].GetComponentsInChildren<Card>();
					foreach(var h in hands){
						if(h.Value==id){
							Destroy(h.gameObject);
							removed=true;
							break;
						}
					}
					if(removed)break;
					
					//remove from melds
					var meldBunch=MeldAreas[i].GetComponentsInChildren<Bunch>();
					foreach(var mb in meldBunch){
						var found=false;
						var val=mb.Value;
						foreach(var b in val.Pawns){
							if(id==b){
								found=true;
								break;
							}
							if(found)break;
						}
						if(found){
							Destroy(mb.gameObject);
							removed=true;
							break;
						}
					}
					if(removed)break;
				}//for(int i=0;i<maxPlayer;++i)
			}//foreach(var id in op.Pawns)

			int fin=0;

			switch(op.Type){
			case pb_enum.BunchA:
				//draw
				foreach(var id in op.Pawns){
					Card.Create(Rule.CardPrefab,id,HandAreas[op.Pos],delegate(Card card) {
						card.state=Card.State.ST_MELD;
						++fin;
					});
				}
				break;
			case pb_enum.OpPass:
				foreach(var id in op.Pawns){
					Card.Create(Rule.CardPrefab,id,AbandonAreas[op.Pos],delegate(Card card) {
						card.state=Card.State.ST_MELD;
						++fin;
					});
				}
				break;
			case pb_enum.BunchWin:
				foreach(Transform ch in HandAreas[op.Pos])Destroy(ch.gameObject);
				foreach(Transform ch in MeldAreas[op.Pos])Destroy(ch.gameObject);
				fin=op.Pawns.Count-op.Child.Count;
				foreach(bunch_t bun in op.Child){
					Rule.LoadBunch(MeldAreas[op.Pos],delegate(Bunch zb) {
						zb.Value=bun;
						++fin;
					});
				}
				break;
			default:
				Rule.LoadBunch(MeldAreas[op.Pos],delegate(Bunch zb) {
					zb.Value=op;
					fin=op.Pawns.Count;
				});
				break;
			}

			while(fin<op.Pawns.Count)yield return null;
		}
		yield break;
	}

	public static void Create(string path,System.Action<Component> handler=null){
		Utils.Load<ReplayPanel>(Main.Instance.RootPanel,delegate(Component obj){
			if(handler!=null)handler.Invoke(obj);
		},path);
	}
}
