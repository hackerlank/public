using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public abstract class SettlePopup : MonoBehaviour {
	public Transform	Items;
	public Transform	Pile;

	virtual public MsgNCSettle Value{
		set{
			foreach(play_t play in value.Play){
				createItem(Items,play);
			}

			//pile
			if(Pile!=null){
				var ctrl=Main.Instance.gameController as GamePanel;
				foreach(var card in value.Pile)
					Card.Create(ctrl.Rule.CardPrefab,card,Pile,delegate(Card obj) {
						obj.state=Card.State.ST_ABANDON;
					});
			}
		}
	}

	abstract protected void createItem(Transform parent,play_t play);
	protected void createItem<T>(Transform parent,play_t play){
		StartCoroutine(Main.Instance.resourceUpdater.Load<T>(
			"Prefabs/"+typeof(T).Name,parent,delegate(Object obj,Hashtable arg) {
			var item=obj as SettleItem;
			item.Value=play;
		}));
	}

	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController.Round>=Main.Round){
			Main.Instance.StartCoroutine(Main.Instance.resourceUpdater.Load<SummaryPanel>(
				"Prefabs/SummaryPanel",Main.Instance.RootPanel));
		}else{
			MsgCNReady msg=new MsgCNReady();
			msg.Mid=pb_msg.MsgCnReady;
			Main.Instance.MainPlayer.Send<MsgCNReady>(msg.Mid,msg);
		}
	}
}
