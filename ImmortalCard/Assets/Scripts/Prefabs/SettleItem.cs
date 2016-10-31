using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class SettleItem : MonoBehaviour {
	public PlayerIcon	Players;
	public Transform	Cards;
	public Text			Point,Score,Achvs;

	public MsgNCSettle Value{
		set{

		}
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
