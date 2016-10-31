using UnityEngine;
using System.Collections;
using Proto3;

public class MahjongSettle : SettlePopup {

	override public MsgNCSettle Value{
		set{
			base.Value=value;
		}
	}
	
	override protected void createItem(Transform parent,play_t play){
		createItem<PaohuziSettle>(parent,play);
	}
}
