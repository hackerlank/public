using UnityEngine;
using System.Collections;
using Proto3;

public class DoudeZhuSettle : SettlePopup {

	override public MsgNCSettle Value{
		set{
			base.Value=value;
		}
	}
	
	override protected void createItem(Transform parent,play_t play){
		createItem<PaohuziSettleItem>(parent,play);
	}
}
