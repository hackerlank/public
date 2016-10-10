using UnityEngine;
using System.Collections;
using Proto3;

public class SettlePopup : MonoBehaviour {

	void Start () {
	
	}
	
	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController.Round>=Main.Instance.Round){
			Utils.Load<SummaryPanel>(Main.Instance.transform);
		}else{
			MsgCNReady msg=new MsgCNReady();
			msg.Mid=pb_msg.MsgCnReady;
			Main.Instance.player.ws.Send<MsgCNReady>(msg.Mid,msg);
		}
	}
}
