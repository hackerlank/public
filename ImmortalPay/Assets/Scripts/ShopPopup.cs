using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ShopPopup: MonoBehaviour{
	public Transform	Items;

	public static ShopPopup Instance;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}

	public MsgPCOrder msgOrder;
	public MsgPCVerify msgVerify;

	public IEnumerator OnBuy(ShopItem item){
		if(null==Main.Instance.MainPlayer.playData.Player){
			Debug.LogError("not logged in");
			yield break;
		}

		var Price=item.Price.text.Substring(1);
		var price=uint.Parse(Price);

		MsgCPOrder msg=new MsgCPOrder();
		msg.Mid=pb_msg.MsgCpOrder;
		msg.Amount=price;
		msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
		msg.Session=Main.Instance.MainPlayer.session;
		msg.Pkcs8=(Application.platform ==RuntimePlatform.Android);

		
		//Debug.Log("----DoLogin account="+msg.User.Account);
		msgOrder=null;
		Main.Instance.MainPlayer.http.Request<MsgCPOrder>(msg.Mid,msg);
		while (msgOrder==null)yield return null;

		Debug.Log("ordered "+price.ToString()+",scheme="+msgOrder.AppScheme+",ostr="+msgOrder.OrderString);
		Main.Instance.alipay.payHandler=delegate(string app_id, string seller_id, string out_trade_no, uint total_amount) {
			StartCoroutine(onPayed(app_id,seller_id,out_trade_no,total_amount));
		};
		Main.Instance.alipay.Pay(msgOrder.AppScheme,msgOrder.OrderString);
	}

	IEnumerator onPayed(string app_id, string seller_id, string out_trade_no, uint total_amount) {
		Debug.Log("charged "+total_amount.ToString());
		MsgCPVerify msg=new MsgCPVerify();
		msg.Mid=pb_msg.MsgCpVerify;
		msg.AppId=app_id;
		msg.OutTradeNo=out_trade_no;
		msg.SellerId=seller_id;
		msg.TotalAmount=total_amount;
		msg.Uid=Main.Instance.MainPlayer.playData.Player.Uid;
		
		msgVerify=null;
		Main.Instance.MainPlayer.http.Request<MsgCPVerify>(msg.Mid,msg);
		while (msgVerify==null)yield return null;
		
		Debug.Log("got gold "+msgVerify.Player.Gold.ToString());
	}

	public void OnClose(){
		Destroy(gameObject);
	}
}
