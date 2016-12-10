using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class ShopItem : MonoBehaviour {
	public Text	LeftPromo,RightPromo;
	public Text	Name,Quantity,Price,Discount;
	public RectTransform GemsImage;
/*
	Product data;
	public Product Data{
		get{return data;}
		set{
			data=value;
			if(data==null){
				Debug.LogError("Assign product: Checked Product outside but still null");
				return;
			}
			// pid=com.happylatte.warnuts.ios.gems45_usd20,title=Bag of 20 Gems,price=19.99,lprice=$19.99,
			//currency=USD,symbol=$,desc=Soft currency for Warnuts 

			currency_t currency=data.serverProduct;
			var title=data.storeProduct.title;
			if(title.Contains("(")&&title.Contains(")"))
				title=title.Substring(0,title.IndexOf("("));
			Name.text=title;
			Quantity.text=currency.Quantity.ToString();
			Price.text=data.storeProduct.localizedPrice;
			if(currency.PromoLeft.Length>0){
//				LeftPromo.text=Game.Localize(hc.PromoLeft);
				LeftPromo.transform.parent.gameObject.SetActive(true);
			}
			if(currency.PromoRight.Length>0){
//				RightPromo.text=Game.Localize(hc.PromoRight);
				RightPromo.transform.parent.gameObject.SetActive(true);
			}
		}
	}
*/
	public void OnClick(){
		if(ShopPopup.Instance!=null)ShopPopup.Instance.OnBuy(this);
	}
}
