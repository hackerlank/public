using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class PaohuziSettle : SettlePopup {
	override public MsgNCSettle Value{
		set{
			base.Value=value;
		}
	}

	override protected void createItem(Transform parent,play_t play){
		createItem<PaohuziSettleItem>(parent,play);
	}
}
