using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahjongSettleItem : SettleItem {
	public Transform		Bunches,Hands;
	public Text				Info;

	override public play_t Value{
		set{
			base.Value=value;

			var bunches=new List<bunch_t>(value.Bunch);
			//hands
			var hands=new List<int>(value.Hands);
			Debug.Log("settle "+value.Seat+" bunches="+bunches.Count+",hands="+hands.Count);

			foreach(var bunch in bunches){
				StartCoroutine(Main.Instance.resourceUpdater.Load<MahjongBunch>(
					"Prefabs/MahjongBunch",Bunches,delegate(Object obj,Hashtable arg) {
					var zi=obj as MahjongBunch;
					zi.Value=bunch;
				}));
			}

			var prefab=(Main.Instance.gameController as GamePanel).Rule.CardPrefab;
			foreach(var hand in hands){
				Card.Create(prefab,hand,Hands,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
				});
			}

			//achievments
			string ach="";
			foreach(var achv in value.Achvs){
				//TODO: phz achivements
				ach+=((int)achv.Type).ToString()+"\n";
				switch(achv.Type){
				case pb_enum.AchvHeaven:
					break;
				}
			}
			
			if(ach.Length>1)Info.text=ach;
		}
	}
}
