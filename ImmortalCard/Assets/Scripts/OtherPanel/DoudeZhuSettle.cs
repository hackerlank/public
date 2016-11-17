using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class DoudeZhuSettle : SettlePopup {
	public Text	Anti,Multiple,Bomb,Multiples;

	override public MsgNCSettle Value{
		set{
			base.Value=value;
		}
	}
	
	override protected void createItem(Transform parent,play_t play){
		createItem<DoudeZhuSettleItem>(parent,play);
	}
}
