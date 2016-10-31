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
				Card.Create(ctrl.CardPrefab,-1,Pile);
			}
		}
	}

	abstract protected void createItem(Transform parent,play_t play);
	protected void createItem<T>(Transform parent,play_t play){
		Utils.Load<T>(parent,delegate(Component obj) {
			var item=obj as SettleItem;
			item.Value=play;
		});
	}

	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController.Round>=Main.Round){
			Utils.Load<SummaryPanel>(Main.Instance.transform);
		}else{
			MsgCNReady msg=new MsgCNReady();
			msg.Mid=pb_msg.MsgCnReady;
			Main.Instance.MainPlayer.Send<MsgCNReady>(msg.Mid,msg);
		}
	}
}
