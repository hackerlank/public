using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class ShopPopup: MonoBehaviour{
	public Transform	Items;

	public static ShopPopup Instance;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	/*
	IEnumerator Start(){
		while(!Main.Instance.billing.StoreLoaded)yield return null;

		for(int i=0;i<Main.Instance.billing.Products.Count;++i){
			Hashtable param=new Hashtable();
			param["data"]=Main.Instance.billing.Products[i];
			StartCoroutine(
				Main.Instance.updater.Load<ShopItem>("Prefabs/ShopItem",Items,delegate(Object arg1, Hashtable arg2) {
				var item=arg1 as ShopItem;
				var product=param["data"] as Product;
				item.Data=product;
			},param));
		}
	}

	public void OnBuy(ShopItem item){
		//buying
		System.Action fly=delegate{
			int gems=int.Parse(item.Quantity.text);
			Main.Instance.billing.GiveGems(gems,item.GemsImage);
		};
		if(item.Data==null)
			Debug.LogError("Purchase: Checked Product outside but still null");
		else
			Main.Instance.billing.Buy(item.Data.id,fly);
	}
*/
	public void OnClose(){
		Destroy(gameObject);
	}
}
