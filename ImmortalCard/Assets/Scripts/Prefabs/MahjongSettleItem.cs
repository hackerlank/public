using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class MahjongSettleItem : SettleItem {
	public Transform		Items;
	public Text				Info;

	override public play_t Value{
		set{
			base.Value=value;

			//achievments
			string ach="";
			foreach(var achv in value.Achvs){
				//TODO: achivements
				ach+=((int)achv.Type).ToString()+"\n";
				switch(achv.Type){
				case pb_enum.AchvHeaven:
					break;
				}
			}

			if(Achvs!=null && ach.Length>1)Achvs.text=ach;
		}
	}
}
