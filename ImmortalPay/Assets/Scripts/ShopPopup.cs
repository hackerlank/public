using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ShopPopup: MonoBehaviour{
	public Transform	Items;

	public static ShopPopup Instance;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	public void OnBuy(ShopItem item){
		var quantity=uint.Parse(item.Quantity.text);

		MsgCPOrder msg=new MsgCPOrder();
		msg.Mid=pb_msg.MsgCpOrder;
		msg.Amount=quantity;
		msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
		
		//Debug.Log("----DoLogin account="+msg.User.Account);
		Main.Instance.MainPlayer.http.Request<MsgCPOrder>(msg.Mid,msg);
	}

	public void OnClose(){
		Destroy(gameObject);
	}
}
