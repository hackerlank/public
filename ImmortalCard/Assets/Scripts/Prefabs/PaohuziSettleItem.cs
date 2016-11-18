using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziSettleItem : SettleItem {
	public Text			Chunk,Multiple;
	public Transform	Bunches;
	public Text			Point,Achvs;

	override public play_t Value{
		set{
			base.Value=value;
			Chunk.text=value.Chunk.ToString();
			Multiple.text=value.Multiple.ToString();
			Point.text=value.Point.ToString();
			Achvs.text="无";

			var bunches=new List<bunch_t>(value.Bunch);
			//hands
			var hands=new List<int>(value.Hands);
			Debug.Log("settle "+value.Seat+" bunches="+bunches.Count+",hands="+hands.Count);
			bunches.AddRange(PaohuziRule.buildFrees(hands,bunches.Count));

			foreach(var bunch in bunches){
				Utils.Load<ZipaiBunch>(Bunches,delegate(Component obj) {
					var zi=obj as ZipaiBunch;
					zi.ShowType=true;
					zi.Value=bunch;
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

			if(ach.Length>1)Achvs.text=ach;
		}
	}
}
