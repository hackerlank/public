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

			//log
			/*
			Debug.Log("bunches: "+Player.bunches2str(bunches));
			string str="hands: ";
			foreach(var hand in value.Hands)str+=hand+",";
			Debug.Log(str);
			*/

			foreach(var bunch in bunches){
				var param=new Hashtable();
				param["bunch"]=bunch;
				StartCoroutine(Main.Instance.updater.Load<MahjongBunch>(
					"Prefabs/MahjongBunch",Bunches,delegate(Object obj,Hashtable arg) {
					var zi=obj as MahjongBunch;
					zi.Value=arg["bunch"] as bunch_t;
				},param));
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
